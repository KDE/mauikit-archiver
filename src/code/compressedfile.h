#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QSettings>

#include <MauiKit4/Core/fmh.h>
#include <MauiKit4/Core/mauilist.h>

class TemporaryFile;
class KArchive;
class KArchiveFile;

class CompressedFile;

class Compressor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString defaultSaveDir READ defaultSaveDir WRITE setDefaultSaveDir NOTIFY defaultSaveDirChanged)

public:
    Compressor(QObject * parent = nullptr);
    ~Compressor();

    QString defaultSaveDir() const;
    void setDefaultSaveDir(QString defaultSaveDir);

public Q_SLOTS:
    bool compress(const QStringList &files, const QUrl &where, const QString &fileName, const int &compressTypeSelected);

private:
    QString m_defaultSaveDir;
    QSettings *m_settings;

Q_SIGNALS:
    void compressionFinished(const QString &url, bool ok);
    void defaultSaveDirChanged(QString defaultSaveDir);
};

class CompressedFileModel : public MauiList
{
    Q_OBJECT

public:
    explicit CompressedFileModel(CompressedFile *parent);

    const FMH::MODEL_LIST &items() const override final;
    void setEntries(FMH::MODEL_LIST list);

private:
    CompressedFile *m_file;
    FMH::MODEL_LIST m_list;
};

class CompressedFile : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(CompressedFileModel *model READ model CONSTANT FINAL)
    Q_PROPERTY(QString currentPath READ currentPath NOTIFY currentPathChanged)
    Q_PROPERTY(QString fileName READ fileName NOTIFY fileNameChanged)
    Q_PROPERTY(bool canGoUp READ canGoUp NOTIFY canGoUpChanged)
    Q_PROPERTY(bool opened READ opened NOTIFY openedChanged)

public:
    explicit CompressedFile(QObject *parent = nullptr);
    ~CompressedFile();

    const KArchiveFile *getFile(const QString &path);

    void setUrl(const QUrl &url);
    QUrl url() const;
    bool isOpen();

    CompressedFileModel *model() const;
    QString currentPath() const;

    QString fileName() const;

    bool canGoUp() const;

    bool opened() const;

    void refresh();

private:
    QUrl m_url;
    QString m_currentPath = "/";
    QString m_fileName;
    bool m_canGoUp = false;
    bool m_opened;

    bool addFile(const QString &url, const QString &path);
    bool extractFile(const QString &url, const QString &where);

    KArchive *m_archive;

    CompressedFileModel *m_model;
    static KArchive *getKArchiveObject(const QUrl &url);
    QHash<QString, TemporaryFile*> m_previews;

public Q_SLOTS:
    void extract(const QUrl &where, const QString &directory = QString());

    void openDir(const QString &path);
    void goUp();
    void goToRoot();
    void close();
    void open();

    QString temporaryFile(const QString &path);

    bool addFiles(const QStringList &urls, const QString &path);

    bool extractFiles(const QStringList &urls, const QString &where);
    bool compress(const QStringList &files, const QUrl &where, const QString &fileName, const int &compressTypeSelected);

    void setCurrentPath(QString currentPath);

Q_SIGNALS:
    void urlChanged();
    void extractionFinished(const QString &url, bool ok);
    void compressionFinished(const QString &url, bool ok);

    void currentPathChanged(QString currentPath);
    void fileNameChanged(QString fileName);
    void canGoUpChanged(bool canGoUp);
    void openedChanged(bool opened);
};

class StaticArchive : public QObject
{
    Q_OBJECT

public:
    StaticArchive(QObject * parent = nullptr);
    static StaticArchive *instance();
    static QObject * qmlInstance(QQmlEngine *, QJSEngine *) {

        auto instance = StaticArchive::instance();
        // if(engine)
        //     instance->setRootComponent(engine->rootContext().contextObject());
        // C++ and QML instance they are the same instance
        return instance;
    }

public Q_SLOTS:
    static bool extract(QUrl url, QUrl where, QString dir);    
};
