#include "moduleinfo.h"
#include "../archiver_version.h"

QString MauiKitArchiver::versionString()
{
    return QStringLiteral(Archiver_VERSION_STRING);
}

QString MauiKitArchiver::buildVersion()
{
    return QStringLiteral(GIT_BRANCH)+QStringLiteral("/")+QStringLiteral(GIT_COMMIT_HASH);
}

KAboutComponent MauiKitArchiver::aboutData()
{
    return KAboutComponent(QStringLiteral("MauiKit Archiver"),
                         QStringLiteral("Archiver management controls."),
                         QStringLiteral(Archiver_VERSION_STRING),
                         QStringLiteral("http://mauikit.org"),
                         KAboutLicense::LicenseKey::LGPL_V3);
}
