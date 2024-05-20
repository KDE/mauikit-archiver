#pragma once

#include <QString>
#include <KAboutData>
#include "archiver_export.h"

/**
 * @brief MauiKit Archiver module information.
 */
namespace MauiKitArchiver
{
   ARCHIVER_EXPORT QString versionString();
   ARCHIVER_EXPORT QString buildVersion();
   ARCHIVER_EXPORT KAboutComponent aboutData();
};
