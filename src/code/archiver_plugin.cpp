// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include <QQmlEngine>
#include <QResource>
#include <QQmlContext>

#include "archiver_plugin.h"
#include "compressedfile.h"

void ArchiverPlugin::registerTypes(const char *uri)
{
    #if defined(Q_OS_ANDROID)
    QResource::registerResource(QStringLiteral("assets:/android_rcc_bundle.rcc"));
    #endif
    
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.mauikit.archiver"));
    
    qmlRegisterType<CompressedFile>(uri, 1, 0, "CompressedFile");
    qmlRegisterSingletonType<StaticArchive>(uri, 1, 0, "StaticArchive", &StaticArchive::qmlInstance);
}

void ArchiverPlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    Q_UNUSED(uri);
    Q_UNUSED(engine);
}

QUrl ArchiverPlugin::componentUrl(const QString &fileName) const
{
    return QUrl(resolveFileUrl(fileName));
}
