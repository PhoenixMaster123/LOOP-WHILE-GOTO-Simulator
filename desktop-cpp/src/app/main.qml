import TheoIDE.Pages
import QtQuick.Controls.Material
import TheoIDE.Persistence
import TheoIDE.Models
import TheoIDE.Controls

ApplicationWindow {
    id: root
    width: ApplicationSettings.windowWidth
    height: ApplicationSettings.windowHeight
    visible: true

    Component.onDestruction: {
        ApplicationSettings.windowWidth = width;
        ApplicationSettings.windowHeight = height;
    }

    palette {
        buttonText: ApplicationSettings.foreground
        text: ApplicationSettings.foreground
        base: ApplicationSettings.background
        dark: ApplicationSettings.foreground
        windowText: ApplicationSettings.foreground
        window: ApplicationSettings.background
    }

    Material.theme: ApplicationSettings.theme
    Material.accent: ApplicationSettings.accent
    Material.background: ApplicationSettings.background
    Material.foreground: ApplicationSettings.foreground
    Material.primary: ApplicationSettings.primary

    property DialogService dialogService: DialogService {}
    property FileSystemService fileSystemService: FileSystemService {
        maxFileSizeBytes: ApplicationSettings.maxReadFileSizeBytes
        blockSizeBytes: ApplicationSettings.blockSizeBytes
    }
    property CompilerService compilerService: CompilerService {}
    property VirtualMachineService virtualMachineService: VirtualMachineService {}

    ApplicationMessageDialog {
        model: MessageDialogModel {
            dialogService: root.dialogService
        }
    }

    StackView {
        id: stackView

        anchors.fill: parent

        initialItem: EditorPage {
            model: EditorModel {
                fileSystemService: root.fileSystemService
                dialogService: root.dialogService
                compilerService: root.compilerService
                virtualMachineService: root.virtualMachineService
                compilationTimeoutMs: ApplicationSettings.compilationTimeoutMs
                executionTimeoutMs: ApplicationSettings.executionTimeoutMs
            }
        }
    }
}
