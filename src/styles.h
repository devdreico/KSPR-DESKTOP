#ifndef STYLES_H
#define STYLES_H

#include <QString>

namespace KSPRStyles {

    QString glassmorphismStyle();
    QString chatStyle();
    QString filePanelStyle();
    QString dashboardStyle();
    QString buttonStyle();
    QString inputStyle();
    QString scrollbarStyle();
    QString tooltipStyle();

    void applyTheme(const QString &theme = "light");

}

#endif // STYLES_H
