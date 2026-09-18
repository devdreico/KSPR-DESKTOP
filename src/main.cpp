#include <QApplication>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include "mainwindow.h"
#include "installer.h"
#include "styles.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("KSPR Desktop");
    app.setOrganizationName("KSPR");
    app.setApplicationVersion("1.0.0");
    app.setWindowIcon(QIcon(":/desktop-app-icon.png"));

    QString appDir = QApplication::applicationDirPath();
    QString cliPath = appDir + "/bin/kspr";

    Installer installer;

    if (!installer.isInstalled(cliPath)) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            nullptr,
            "KSPR CLI no encontrado",
            "KSPR CLI no está instalado.\n\n"
            "¿Deseas instalarlo ahora?\n"
            "Se descargará desde GitHub.",
            QMessageBox::Yes | QMessageBox::No
        );

        if (reply == QMessageBox::Yes) {
            installer.setInstallDir(appDir + "/bin");
            installer.install();

            if (!installer.isInstalled(cliPath)) {
                QMessageBox::critical(
                    nullptr,
                    "Error de instalación",
                    "No se pudo instalar KSPR CLI.\n"
                    "Por favor, instala manualmente:\n\n"
                    "curl -sSL https://raw.githubusercontent.com/devdreiortiz/KSPR/main/bin/install.sh | bash"
                );
                return 1;
            }
        } else {
            QMessageBox::information(
                nullptr,
                "Instalación requerida",
                "KSPR CLI es necesario para usar la aplicación.\n"
                "Instálalo manualmente y vuelve a iniciar."
            );
            return 1;
        }
    }

    MainWindow window(cliPath);
    window.show();

    return app.exec();
}
