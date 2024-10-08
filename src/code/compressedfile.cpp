#include "compressedfile.h"

#include <KTar>
#include <KZip>
#include <KAr>

#if (defined Q_OS_LINUX || defined Q_OS_FREEBSD) && !defined Q_OS_ANDROID
#include <K7Zip>
#endif

#include <QDirIterator>
#include <QDebug>

#include <MauiKit4/FileBrowsing/fmstatic.h>

#include "temporaryfile.h"


CompressedFile::CompressedFile(QObject *parent)
    : QObject(parent)
    , m_archive(nullptr)
    , m_model(new CompressedFileModel(this))
{
}

CompressedFileModel::CompressedFileModel(CompressedFile *parent)
    : MauiList(parent)
    , m_file(parent)
{
}

CompressedFile::~CompressedFile()
{
    qDeleteAll(m_previews);
    if(m_archive)
    {
        m_archive->close();
        delete m_archive;
    }
}

const FMH::MODEL_LIST &CompressedFileModel::items() const
{
    return m_list;
}

void CompressedFileModel::setEntries(FMH::MODEL_LIST list)
{
    m_list.clear();

    Q_EMIT this->preListChanged();

    m_list = list;

    Q_EMIT this->postListChanged();
    Q_EMIT this->countChanged ();
}

QString CompressedFile::currentPath() const
{
    return m_currentPath;
}

QString CompressedFile::fileName() const
{
    return m_fileName;
}

bool CompressedFile::canGoUp() const
{
    return m_canGoUp;
}

bool CompressedFile::opened() const
{
    return m_opened;
}

void CompressedFile::refresh()
{
    openDir(m_currentPath);
}

const KArchiveFile *CompressedFile::getFile(const QString &path)
{
    if(!m_archive)
    {
        return nullptr;
    }

    if(m_archive->isOpen())
    {
        return m_archive->directory()->file(path);
    }

    return nullptr;
}

void CompressedFile::openDir(const QString &path)
{
    if(m_url.isEmpty() || path.isEmpty() || !m_archive)
    {
        return;
    }

    if(m_archive->isOpen())
    {
        auto root = m_archive->directory();
        auto entry = root->entry(path);

        if(entry)
        {
            if(entry->isDirectory())
            {
                const KArchiveDirectory* subDir = static_cast<const KArchiveDirectory*>(entry);
                setCurrentPath(path);

                FMH::MODEL_LIST list;
                for(const auto &file : subDir->entries())
                {
                    const auto e = subDir->entry(file);
                    list << FMH::MODEL{{FMH::MODEL_KEY::IS_DIR, e->isDirectory() ? "true" : "false"}, {FMH::MODEL_KEY::LABEL, e->name()}, {FMH::MODEL_KEY::ICON, e->isDirectory() ? "folder" : FMStatic::getIconName(e->name())}, {FMH::MODEL_KEY::DATE, e->date().toString()}, {FMH::MODEL_KEY::PATH, QString(path+"%1/").arg(e->name())}, {FMH::MODEL_KEY::USER, e->user()}};

                }

                m_model->setEntries(list);
            }
        }
    }
}

void CompressedFile::goUp()
{
    this->openDir(QUrl(m_currentPath).resolved(QUrl("..")).toString());
}

void CompressedFile::goToRoot()
{
    this->openDir("/");
}

void CompressedFile::close()
{
    m_archive->close();
}

void CompressedFile::open()
{
    m_archive = CompressedFile::getKArchiveObject(m_url);
    m_archive->open(QIODevice::ReadOnly);

    assert(m_archive->isOpen() == true);

    m_opened = m_archive->isOpen();
    Q_EMIT openedChanged(m_opened);
}

QString CompressedFile::temporaryFile(const QString &path)
{
    if(m_previews.contains(path))
    {
        return m_previews.value(path)->url();
    }

    auto preview = new TemporaryFile;
    m_previews.insert(path, preview);

    auto file = getFile(path);
    preview->setData(file->data(), file->name());
    return preview->url();
}

bool CompressedFile::addFiles(const QStringList &urls, const QString &path)
{
    if(urls.isEmpty() || path.isEmpty())
    {
        return false;
    }

    bool success = false;

    m_archive->close();
    m_archive->open(QIODevice::ReadWrite);

    for(const auto &url : urls)
    {
        success = addFile(url, path);
    }

    m_archive->close();
    m_archive->open(QIODevice::ReadOnly);
    refresh();

    return success;
}

bool CompressedFile::addFile(const QString &url, const QString &path)
{
    auto localUrl = QUrl(url).toLocalFile();
    QFileInfo file(localUrl);

    if(!file.exists())
    {
        return false;
    }

    if(file.isDir())
    {
        return m_archive->addLocalDirectory(localUrl, path+file.fileName());
    }


    if(m_archive->addLocalFile(localUrl, path+file.fileName()))
    {
        qDebug() << "Trying to insert file to archive"<< url << localUrl << path << path+file.fileName();
        return true;
    }

    return false;
}

bool CompressedFile::extractFiles(const QStringList &urls, const QString &where)
{
    return false;
}

bool CompressedFile::compress(const QStringList &files, const QUrl &where, const QString &fileName, const int &compressTypeSelected)
{
    auto compressor = new Compressor();
    connect(compressor, &Compressor::compressionFinished, this, [this, compressor](QString url, bool ok)
            {
                if(ok)
                    this->setUrl(url);

                Q_EMIT compressionFinished(url, ok);
                compressor->deleteLater();
            });
    return compressor->compress(files, where, fileName, compressTypeSelected);
}

bool CompressedFile::extractFile(const QString &url, const QString &where)
{
    return false;
}

void CompressedFile::setCurrentPath(QString currentPath)
{
    if (m_currentPath == currentPath)
        return;

    m_currentPath = currentPath;
    Q_EMIT currentPathChanged(m_currentPath);

    m_canGoUp = m_currentPath != "/";
    Q_EMIT canGoUpChanged(m_canGoUp);
}

void CompressedFile::extract(const QUrl &where, const QString &directory)
{
    if (!m_url.isLocalFile())
        return;    

    if(!m_archive)
        return;

    qDebug() << "@gadominguez File:fm.cpp Funcion: extractFile  "
             << "URL: " << m_url << "WHERE: " << where.toString() << " DIR: " << directory;

    QString where_ = where.toLocalFile() + "/" + directory;

           // auto kArch = CompressedFile::getKArchiveObject(m_url);
           // kArch->open(QIODevice::ReadOnly);

    qDebug() << "@gadominguez File:fm.cpp Funcion: extractFile  " << m_archive->directory()->entries();

    bool ok = false;
    if (m_archive->isOpen())
    {
        ok = m_archive->directory()->copyTo(where_, true);
    }

    Q_EMIT this->extractionFinished(where.toString(), ok);
}

/*
 *
 *  CompressTypeSelected is an integer and has to be acorrding with order in Dialog.qml
 *
 */
Compressor::Compressor(QObject *parent) : QObject(parent)
    ,m_defaultSaveDir(FMStatic::DocumentsPath)
    ,m_settings(new QSettings(QStringLiteral("org.mauikit.archiver"), "", this))
{
    m_settings->beginGroup("General");
    m_defaultSaveDir = m_settings->value("DefaultSaveDir", m_defaultSaveDir).toString();
    m_settings->endGroup();
}

static std::vector<std::string> QStringList_to_VectorString(const QList<QString>& qlist) {
    std::vector<std::string> result(qlist.size());
    for (int i=0; i<qlist.size(); i++) {
        result[i] = qlist.at(i).toUtf8().data();
    }
    return result;
}

bool Compressor::compress(const QStringList &files, const QUrl &where, const QString &fileName, const int &compressTypeSelected)
{
    QString commonPath = "";
    auto fileWriter = [&commonPath](KArchive *archive, QFile &file) -> bool
    {
        if(!archive)
            return false;

        return archive->writeFile(file.fileName().remove(commonPath, Qt::CaseSensitivity::CaseSensitive), // Mirror file path in compressed file from current directory
                                  file.readAll(),
                                  0100775,
                                  QFileInfo(file).owner(),
                                  QFileInfo(file).group(),
                                  QDateTime(),
                                  QDateTime(),
                                  QDateTime());
    };

    auto dirWriter = [&fileWriter](KArchive *archive, QDirIterator &dir) -> bool
    {
        bool ok = false;

        if(!archive)            
        {
            return ok;
        }

        while (dir.hasNext())
        {
            auto entrie = dir.next();

            if (QFileInfo(entrie).isFile())
            {
                auto file = QFile(entrie);
                file.open(QIODevice::ReadOnly);

                if (!file.isOpen())
                {
                    qDebug() << "ERROR. CURRENT USER DOES NOT HAVE PEMRISSION FOR WRITE IN THE CURRENT DIRECTORY.";
                    continue;
                }

                ok = fileWriter(archive, file);
                       // WriteFile returns if the file was written or not,
                       // but this function returns if some error occurs so for this reason it is needed to toggle the value
            }
        }

        return ok;
    };

    auto url = [&where, &fileName](const int &type) -> QString
    {
        QString format;
        switch(type)
        {
        case 0: format = ".zip"; break;
        case 1: format = ".tar"; break;
        case 2: format = ".7zip"; break;
        case 3: format = ".ar"; break;
        }

        return QUrl(where.toString() + "/" + fileName + format).toLocalFile();
    };

    auto commonPathFunc = [] (const std::vector<std::string> & dirs) -> std::string
    {
        std::vector<std::string>::const_iterator vsi = dirs.begin( ) ;
        int maxCharactersCommon = vsi->length( ) ;
        std::string compareString = *vsi ;
        for ( vsi = dirs.begin( ) + 1 ; vsi != dirs.end( ) ; vsi++ ) {
            std::pair<std::string::const_iterator , std::string::const_iterator> p =
                std::mismatch( compareString.begin( ) , compareString.end( ) , vsi->begin( ) ) ;
            if (( p.first - compareString.begin( ) ) < maxCharactersCommon )
                maxCharactersCommon = p.first - compareString.begin( ) ;
        }
        std::string::size_type found = compareString.rfind( '/' , maxCharactersCommon ) ;
        return compareString.substr( 0 , found ) ;
    };

    commonPath = QString::fromStdString(commonPathFunc( QStringList_to_VectorString(files) )).remove("file://");

    qDebug() << "The Common path is << " << commonPath;

    auto openCheck = [](KArchive *archive) -> bool
    {
        if(!archive)
            return false;

        archive->open(QIODevice::ReadWrite);
        return archive->isOpen();
    };

    bool ok = false;
    const QString fileUrl = url(compressTypeSelected);

    assert(compressTypeSelected >= 0 && compressTypeSelected <= 8);

    KArchive *archive = nullptr;

    switch (compressTypeSelected)
    {
    case 0: //.ZIP
    {
        archive = new KZip(fileUrl);
        break;
    }
    case 1: // .TAR
    {
        archive = new KTar(fileUrl);
        break;
    }
    case 2: //.7ZIP
    {
#ifdef K7ZIP_H

               // TODO: KArchive no permite comprimir ficheros del mismo modo que con TAR o ZIP. Hay que hacerlo de otra forma y requiere disponer de una libreria actualizada de KArchive.
        archive = new K7Zip(fileUrl);
#endif
        break;
    }
    case 3: //.AR
    {
        // TODO: KArchive no permite comprimir ficheros del mismo modo que con TAR o ZIP. Hay que hacerlo de otra forma y requiere disponer de una libreria actualizada de KArchive.
        archive = new KAr(fileUrl);
        break;
    }
    default:
        qDebug() << "ERROR. COMPRESSED TYPE SELECTED NOT COMPATIBLE";
        break;
    }

    if(!openCheck(archive))
    {
        ok = false;
        Q_EMIT compressionFinished(QUrl::fromLocalFile(fileUrl).toString(), ok);
        return ok;
    }

    for (const auto &uri : files)
    {
        qDebug() << "@gadominguez File:fm.cpp Funcion: compress  " << QUrl(uri).toLocalFile() << " " << fileName;

        if (!QFileInfo(QUrl(uri).toLocalFile()).isDir())
        {
            auto file = QFile(QUrl(uri).toLocalFile());
            file.open(QIODevice::ReadWrite);

            if (!file.isOpen())
            {
                qDebug() << "ERROR. CURRENT USER DOES NOT HAVE PEMRISSION FOR WRITE IN THE CURRENT DIRECTORY.";
                ok = false;
                continue;
            }

            ok = fileWriter(archive, file);

        } else
        {                        
            qDebug() << "Dir: " << QUrl(uri).toLocalFile();
            auto dir = QDirIterator(QUrl(uri).toLocalFile(), QDirIterator::Subdirectories);
            ok = dirWriter(archive, dir);
        }
    }

    (void)archive->close();
                            // kzip->prepareWriting("Hello00000.txt", "gabridc", "gabridc", 1024, 0100777, QDateTime(), QDateTime(), QDateTime());
                            // kzip->writeData("Hello", sizeof("Hello"));
                            // kzip->finishingWriting();

    Q_EMIT compressionFinished(QUrl::fromLocalFile(fileUrl).toString(), ok);
    return ok;
}

KArchive *CompressedFile::getKArchiveObject(const QUrl &url)
{
    KArchive *kArch = nullptr;

    /*
     * This checks depends on type COMPRESSED_MIMETYPES in file fmh.h
     */
    qDebug() << "@gadominguez File: fmstatic.cpp Func: getKArchiveObject MimeType: " << FMStatic::getMime(url);

    auto mime = FMStatic::getMime(url);

    if (mime.contains("application/x-tar") || mime.contains("application/x-compressed-tar"))
    {
        kArch = new KTar(url.toLocalFile());
    } else if (mime.contains("application/zip"))
    {
        kArch = new KZip(url.toLocalFile());

    } else if (mime.contains("application/x-7z-compressed"))
    {
#ifdef K7ZIP_H
        kArch = new K7Zip(url.toLocalFile());
#endif
    } else
    {
        qDebug() << "ERROR. COMPRESSED FILE TYPE UNKOWN " << url.toString();
    }

    return kArch;
}

void CompressedFile::setUrl(const QUrl &url)
{
    if (m_url == url)
        return;

    m_url = url;
    Q_EMIT this->urlChanged();

    if(!FMStatic::fileExists(m_url))
    {
        qWarning()<< "File does not exists and can not be opened." << url;
        return;
    }

    m_fileName = QFileInfo(m_url.toLocalFile()).baseName();
    Q_EMIT fileNameChanged(m_fileName);

    open();
    openDir(m_currentPath);
}

QUrl CompressedFile::url() const
{
    return m_url;
}

bool CompressedFile::isOpen()
{
    if(!m_archive)
        return false;

    return m_archive->isOpen();
}

CompressedFileModel *CompressedFile::model() const
{
    return m_model;
}


Q_GLOBAL_STATIC(StaticArchive, appInstance)
StaticArchive::StaticArchive(QObject *parent) : QObject(parent)
{
}

StaticArchive *StaticArchive::instance()
{
    return appInstance();
}

QString Compressor::defaultSaveDir() const
{
    return m_defaultSaveDir;
}

void Compressor::setDefaultSaveDir(QString defaultSaveDir)
{
    if (m_defaultSaveDir == defaultSaveDir)
        return;

    m_defaultSaveDir = defaultSaveDir;

    m_settings->beginGroup("General");
    m_settings->setValue("DefaultSaveDir", m_defaultSaveDir);
    m_settings->endGroup();

    Q_EMIT defaultSaveDirChanged(m_defaultSaveDir);
}

Compressor::~Compressor()
{
    m_settings->sync();
}

bool StaticArchive::extract(QUrl url, QUrl where, QString dir)
{
    CompressedFile file;
    file.setUrl(url);

    if(!file.isOpen())
        return false;

    file.extract(where, dir);
    return true;
}
