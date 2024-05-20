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

    // qmlRegisterSingletonType<MauiAccounts>(uri, 1, 0, "Accounts", [](QQmlEngine *engine, QJSEngine *scriptEngine) -> QObject * {
    //     Q_UNUSED(scriptEngine)
    //     auto accounts = MauiAccounts::instance();
    //     engine->setObjectOwnership(accounts, QQmlEngine::CppOwnership);
    //     return accounts;
    // });
    
   qmlRegisterType<CompressedFile>(uri, 1, 0, "CompressedFile");

    
    // qmlRegisterType(componentUrl(QStringLiteral("AccountsMenuItem.qml")), uri, 1, 0, "AccountsMenuItem");
    // qmlRegisterType(componentUrl(QStringLiteral("AccountsDialog.qml")), uri, 1, 0, "AccountsDialog");
    // qmlRegisterType(componentUrl(QStringLiteral("CredentialsDialog.qml")), uri, 1, 0, "CredentialsDialog");
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
