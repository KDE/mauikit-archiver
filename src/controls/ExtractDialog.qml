import QtQuick
import QtCore

import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui
import org.mauikit.archiver as Arc
import org.mauikit.filebrowsing as FB

Maui.InputDialog
{
    id: control

    property url destination
    property url fileUrl
    property string dirName

    textEntry.text: dirName

    title: i18n("Extract")
    message: i18n("Extract the contents of the compressed file")

    Pane
    {
        Layout.fillWidth: true

        background: Rectangle
        {
            color: Maui.Theme.alternateBackgroundColor
            radius: Maui.Style.radiusV
        }

        contentItem: Column
        {
            Label
            {
                width: parent.width
                text: control.fileUrl
                font.family: "Monospace"
                wrapMode: Text.Wrap
                background: Rectangle
                {
                    color: Maui.Theme.alternateBackgroundColor
                    radius: Maui.Style.radiusV
                }
            }

            Label
            {
                 width: parent.width
                text: "=>"
                font.family: "Monospace"
            }

            Label
            {
                 width: parent.width
                text: control.destination
                font.family: "Monospace"
                wrapMode: Text.Wrap

                background: Rectangle
                {
                    color: Maui.Theme.alternateBackgroundColor
                    radius: Maui.Style.radiusV
                }
            }
        }
    }

    onFinished: (text) =>
                {
                    Arc.StaticArchive.extract(control.fileUrl, control.destination, text)
                }

    readonly property bool dirExists: FB.FM.fileExists(control.destination+"/"+control.textEntry.text)
    onDirExistsChanged:
    {
        if(dirExists)
            control.alert(i18n("A directory with the same name already exists!"), 2)
        else
        {
            control.alert(i18n("The name looks good"), 0)
        }
    }

}
