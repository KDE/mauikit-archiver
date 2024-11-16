import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.mauikit.controls as Maui
import org.mauikit.filebrowsing as FM

import org.mauikit.archiver as Arc

FM.FileListingDialog
{
    id: control

    onOpened: _archiveNameField.forceActiveFocus()

    signal done(var files, string path, string name, int type)

    /**
      *@brief The preferred location where the final archive file will be saved at
      */
    property string destination : _compressor.defaultSaveDir

    /**
      *@brief The type fo archive. The possible options are:
      0 - ZIP
      1 - TAR
      2 - 7ZIP
      3 - AR
      */
    property int type : 0
    onTypeChanged:
    {
        control.checkExistance(_archiveNameField.text, _locationField.text, control.extensionName(control.type))
    }

    persistent: false

    message: i18np("Compress %1 file into a new archive", "Compress %1 files into a new archive", urls.length)

    actions:[
        Action
        {
            text: i18n("Cancel")
            Maui.Controls.status: Maui.Controls.Negative
            onTriggered:
            {
                control.clear()
                close()
            }
        },
        Action
        {
            text: i18n("Create")
            Maui.Controls.status: Maui.Controls.Positive
            onTriggered:
            {
                const ok = control.checkExistance(_archiveNameField.text, _locationField.text, control.extensionName(control.type))

                if(!ok)
                {
                    control.alert(i18n("Some error occured. Maybe current user does not have permission for writing in this directory."))
                    return
                }else
                {
                    control.done(control.urls, _locationField.text, _archiveNameField.text, control.type)
                    //            control.close()
                }
            }
        }
    ]

    Action
    {
        property url url
        id: _compressSuccessAction
        text: i18n("Open")
        onTriggered: FM.FM.openUrl(url)
    }

    Arc.Compressor
    {
        id: _compressor
        onCompressionFinished: (url, ok) =>
                               {
                                   if(ok)
                                   {
                                       control.clear()
                                       control.close()
                                   }else
                                   {
                                       return
                                   }

                                    _compressSuccessAction.url = url
                                   Maui.App.rootComponent.notify("application-x-archive",
                                                                 ok ? i18n("File compressed successfully") : i18n("Failed to compress"),
                                                                 url,
                                                                 [_compressSuccessAction])
                               }
    }

    Maui.TextField
    {
        id: _archiveNameField
        Layout.fillWidth: true
        Maui.Controls.title: i18n("Archive name")

        onTextChanged:
        {
            control.checkExistance(text, _locationField.text, control.extensionName(control.type))
        }
    }

    Maui.TextField
    {
        id: _locationField
        Layout.fillWidth: true
        Maui.Controls.title: i18n("Destination")
        Maui.Controls.subtitle: i18n("The final location of the new archive")
        text: control.destination

        onTextChanged:
        {
            control.checkExistance(_archiveNameField.text, text, control.extensionName(control.type))
        }
    }

    Maui.ToolActions
    {
        id: compressType
        autoExclusive: true
        expanded: true
        display: ToolButton.TextBesideIcon

        Action
        {
            text: ".ZIP"
            icon.name:  "application-zip"
            checked: control.type === 0
            onTriggered: control.type = 0
        }

        Action
        {
            text: ".TAR"
            icon.name:  "application-x-tar"
            checked: control.type === 1
            onTriggered: control.type = 1
        }

        Action
        {
            text: ".7ZIP"
            icon.name:  "application-x-rar"
            checked: control.type === 2
            onTriggered: control.type = 2
        }
    }

    function checkExistance(name, path, extension)
    {
        if(!FM.FM.fileExists(path))
        {
            control.alert(i18n("Base location does not exists. Try with a different location."), 2)
            return false
        }

        if(name.length === 0)
        {
            control.alert(i18n("File name can not be empty."), 2)
            return false
        }

        var file = path+"/"+name+extension
        var exists = FM.FM.fileExists(file)

        if(exists)
        {
            control.alert(i18n("File already exists. Try with another name."), 1)
            return false
        }

        control.alert(i18n("Looks good"), 0)
        return true
    }

    function extensionName(type)
    {
        var extension = ""
        switch(control.type)
        {
        case 0: extension = ".zip"; break;
        case 1: extension = ".tar"; break;
        case 2: extension = ".7zip"; break;
        }
        return extension;
    }

    function compress()
    {
        _compressor.compress(control.urls, _locationField.text, _archiveNameField.text, control.type)
    }

    function clear()
    {
        _archiveNameField.clear()
        control.urls = []
    }

}

