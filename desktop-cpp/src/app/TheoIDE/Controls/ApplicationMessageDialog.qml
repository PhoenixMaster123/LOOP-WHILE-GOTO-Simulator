import QtQuick.Dialogs
import TheoIDE.Models
import TheoIDE.Controls

MessageDialog {
    id: root

    required property MessageDialogModel model

    title: model.title
    text: model.text
    informativeText: model.informativeText
    detailedText: model.detailedText
    buttons: dialogButtonsToMessageDialogFlags(model.dialogButtons)

    onButtonClicked: function (dialogFlag, role) {
        model.runButtonAction(dialogFlagToDialogButton(dialogFlag));
    }

    function updateModelOpen(): void {
        model.open = root.visible;
    }

    Component.onCompleted: {
        root.visibleChanged.connect(updateModelOpen);
        model.hasMessageToShow.connect(root.open);
    }

    function dialogFlagToDialogButton(flag: int): int {
        switch (flag) {
        case MessageDialog.Ok:
            return DialogButton.Ok;
        case MessageDialog.Open:
            return DialogButton.Open;
        case MessageDialog.Save:
            return DialogButton.Save;
        case MessageDialog.Cancel:
            return DialogButton.Cancel;
        case MessageDialog.Close:
            return DialogButton.Cancel;
        case MessageDialog.Discard:
            return DialogButton.Discard;
        case MessageDialog.Apply:
            return DialogButton.Apply;
        case MessageDialog.Reset:
            return DialogButton.Reset;
        case MessageDialog.RestoreDefaults:
            return DialogButton.RestoreDefaults;
        case MessageDialog.Help:
            return DialogButton.Help;
        case MessageDialog.SaveAll:
            return DialogButton.SaveAll;
        case MessageDialog.Yes:
            return DialogButton.Yes;
        case MessageDialog.YesToAll:
            return DialogButton.YesToAll;
        case MessageDialog.No:
            return DialogButton.No;
        case MessageDialog.NoToAll:
            return DialogButton.NoToAll;
        case MessageDialog.Abort:
            return DialogButton.Abort;
        case MessageDialog.Retry:
            return DialogButton.Retry;
        case MessageDialog.Ignore:
            return DialogButton.Ignore;
        case MessageDialog.NoButton:
            return DialogButton.NoButton;
        }
        return DialogButton.NoButton;
    }

    function dialogButtonsToMessageDialogFlags(dialogButtons): int {
        let flags = 0;
        for (let index in dialogButtons) {
            let dialogButton = dialogButtons[index];
            switch (dialogButton) {
            case DialogButton.Ok:
                flags |= MessageDialog.Ok;
                break;
            case DialogButton.Open:
                flags |= MessageDialog.Open;
                break;
            case DialogButton.Save:
                flags |= MessageDialog.Save;
                break;
            case DialogButton.Cancel:
                flags |= MessageDialog.Cancel;
                break;
            case DialogButton.Close:
                flags |= MessageDialog.Close;
                break;
            case DialogButton.Discard:
                flags |= MessageDialog.Discard;
                break;
            case DialogButton.Apply:
                flags |= MessageDialog.Apply;
                break;
            case DialogButton.Reset:
                flags |= MessageDialog.Reset;
                break;
            case DialogButton.RestoreDefaults:
                flags |= MessageDialog.RestoreDefaults;
                break;
            case DialogButton.Help:
                flags |= MessageDialog.Help;
                break;
            case DialogButton.SaveAll:
                flags |= MessageDialog.SaveAll;
                break;
            case DialogButton.Yes:
                flags |= MessageDialog.Yes;
                break;
            case DialogButton.YesToAll:
                flags |= MessageDialog.YesToAll;
                break;
            case DialogButton.No:
                flags |= MessageDialog.No;
                break;
            case DialogButton.NoToAll:
                flags |= MessageDialog.NoToAll;
                break;
            case DialogButton.Abort:
                flags |= MessageDialog.Abort;
                break;
            case DialogButton.Retry:
                flags |= MessageDialog.Retry;
                break;
            case DialogButton.Ignore:
                flags |= MessageDialog.Ignore;
                break;
            case DialogButton.NoButton:
                flags |= MessageDialog.NoButton;
                break;
            }
        }
        return flags;
    }
}
