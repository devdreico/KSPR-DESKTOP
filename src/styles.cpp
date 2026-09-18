#include "styles.h"
#include <QApplication>
#include <QPalette>
#include <QColor>

namespace KSPRStyles {

QString glassmorphismStyle()
{
    return R"(
        QMainWindow {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #f3f2ee, stop:1 #e8e4dc);
        }

        QWidget[class="glass"] {
            background: rgba(255, 254, 251, 0.7);
            border: 1px solid rgba(255, 255, 255, 0.3);
            border-radius: 12px;
        }

        QWidget[class="glass-dark"] {
            background: rgba(26, 26, 31, 0.8);
            border: 1px solid rgba(255, 255, 255, 0.1);
            border-radius: 12px;
        }

        QFrame[class="card"] {
            background: rgba(255, 254, 251, 0.7);
            border: 1px solid rgba(255, 255, 255, 0.3);
            border-radius: 12px;
            padding: 16px;
        }

        QFrame[class="sidebar"] {
            background: rgba(243, 242, 238, 0.94);
            border-right: 1px solid #d8d7d1;
        }
    )";
}

QString chatStyle()
{
    return R"(
        QTextEdit[class="chat-display"] {
            background: transparent;
            border: none;
            font-family: 'Montserrat', sans-serif;
            font-size: 14px;
            color: #141414;
            padding: 16px;
        }

        QLineEdit[class="chat-input"] {
            background: rgba(255, 254, 251, 0.7);
            border: 1px solid #d8d7d1;
            border-radius: 12px;
            padding: 12px 16px;
            font-family: 'Montserrat', sans-serif;
            font-size: 14px;
            color: #141414;
        }

        QLineEdit[class="chat-input"]:focus {
            border-color: #6366f1;
            background: rgba(255, 254, 251, 0.9);
        }

        QPushButton[class="send-button"] {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #6366f1, stop:1 #4f46e5);
            color: white;
            border: none;
            border-radius: 8px;
            padding: 10px 20px;
            font-weight: bold;
        }

        QPushButton[class="send-button"]:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #818cf8, stop:1 #6366f1);
        }

        QPushButton[class="send-button"]:pressed {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #4f46e5, stop:1 #4338ca);
        }
    )";
}

QString filePanelStyle()
{
    return R"(
        QListWidget[class="file-list"] {
            background: rgba(255, 254, 251, 0.5);
            border: 1px solid #d8d7d1;
            border-radius: 8px;
            padding: 8px;
            outline: none;
        }

        QListWidget[class="file-list"]::item {
            background: transparent;
            border: none;
            border-radius: 6px;
            padding: 8px 12px;
            margin: 2px 0;
        }

        QListWidget[class="file-list"]::item:selected {
            background: rgba(99, 102, 241, 0.1);
            color: #6366f1;
        }

        QListWidget[class="file-list"]::item:hover {
            background: rgba(99, 102, 241, 0.05);
        }

        QTextEdit[class="file-preview"] {
            background: rgba(255, 254, 251, 0.5);
            border: 1px solid #d8d7d1;
            border-radius: 8px;
            font-family: 'DM Mono', monospace;
            font-size: 12px;
            color: #141414;
            padding: 16px;
        }
    )";
}

QString dashboardStyle()
{
    return R"(
        QFrame[class="metric-card"] {
            background: rgba(255, 254, 251, 0.7);
            border: 1px solid rgba(255, 255, 255, 0.3);
            border-radius: 12px;
            padding: 20px;
        }

        QLabel[class="metric-value"] {
            font-size: 28px;
            font-weight: bold;
            color: #141414;
        }

        QLabel[class="metric-label"] {
            font-family: 'DM Mono', monospace;
            font-size: 10px;
            color: #777873;
            letter-spacing: 0.05em;
        }

        QProgressBar[class="progress-bar"] {
            border: none;
            border-radius: 4px;
            background: #d8d7d1;
            height: 8px;
        }

        QProgressBar[class="progress-bar"]::chunk {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #6366f1, stop:1 #818cf8);
            border-radius: 4px;
        }
    )";
}

QString buttonStyle()
{
    return R"(
        QPushButton {
            font-family: 'DM Mono', monospace;
            font-size: 12px;
            border-radius: 8px;
            padding: 8px 16px;
        }

        QPushButton[class="primary"] {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #6366f1, stop:1 #4f46e5);
            color: white;
            border: none;
        }

        QPushButton[class="primary"]:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #818cf8, stop:1 #6366f1);
        }

        QPushButton[class="secondary"] {
            background: transparent;
            color: #141414;
            border: 1px solid #d8d7d1;
        }

        QPushButton[class="secondary"]:hover {
            border-color: #6366f1;
            color: #6366f1;
        }

        QPushButton[class="ghost"] {
            background: transparent;
            color: #777873;
            border: none;
        }

        QPushButton[class="ghost"]:hover {
            color: #6366f1;
            background: rgba(99, 102, 241, 0.05);
        }
    )";
}

QString inputStyle()
{
    return R"(
        QLineEdit, QTextEdit, QSpinBox, QDoubleSpinBox {
            background: rgba(255, 254, 251, 0.7);
            border: 1px solid #d8d7d1;
            border-radius: 8px;
            padding: 10px 14px;
            font-family: 'Montserrat', sans-serif;
            font-size: 13px;
            color: #141414;
        }

        QLineEdit:focus, QTextEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus {
            border-color: #6366f1;
            background: rgba(255, 254, 251, 0.9);
        }

        QComboBox {
            background: rgba(255, 254, 251, 0.7);
            border: 1px solid #d8d7d1;
            border-radius: 8px;
            padding: 8px 12px;
            font-family: 'Montserrat', sans-serif;
            font-size: 13px;
            color: #141414;
        }

        QComboBox:hover {
            border-color: #6366f1;
        }

        QComboBox::drop-down {
            border: none;
            width: 24px;
        }

        QComboBox QAbstractItemView {
            background: rgba(255, 254, 251, 0.95);
            border: 1px solid #d8d7d1;
            border-radius: 8px;
            selection-background-color: rgba(99, 102, 241, 0.1);
            selection-color: #6366f1;
        }
    )";
}

QString scrollbarStyle()
{
    return R"(
        QScrollBar:vertical {
            background: transparent;
            width: 8px;
            margin: 0;
        }

        QScrollBar::handle:vertical {
            background: #d8d7d1;
            border-radius: 4px;
            min-height: 30px;
        }

        QScrollBar::handle:vertical:hover {
            background: #aaa9a4;
        }

        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0;
        }

        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
            background: transparent;
        }

        QScrollBar:horizontal {
            background: transparent;
            height: 8px;
            margin: 0;
        }

        QScrollBar::handle:horizontal {
            background: #d8d7d1;
            border-radius: 4px;
            min-width: 30px;
        }

        QScrollBar::handle:horizontal:hover {
            background: #aaa9a4;
        }

        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
            width: 0;
        }

        QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {
            background: transparent;
        }
    )";
}

QString tooltipStyle()
{
    return R"(
        QToolTip {
            background: rgba(26, 26, 31, 0.95);
            color: #f0f0ed;
            border: none;
            border-radius: 6px;
            padding: 8px 12px;
            font-family: 'DM Mono', monospace;
            font-size: 11px;
        }
    )";
}

void applyTheme(const QString &theme)
{
    QString styleSheet;

    if (theme == "dark") {
        styleSheet = R"(
            QMainWindow {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                    stop:0 #0f0f12, stop:1 #1a1a1f);
            }

            QWidget {
                color: #f0f0ed;
            }

            QWidget[class="glass"] {
                background: rgba(26, 26, 31, 0.8);
                border: 1px solid rgba(255, 255, 255, 0.1);
            }

            QFrame[class="sidebar"] {
                background: rgba(15, 15, 18, 0.94);
                border-right: 1px solid #2e2e33;
            }

            QLineEdit, QTextEdit, QSpinBox, QDoubleSpinBox {
                background: rgba(26, 26, 31, 0.8);
                border: 1px solid #2e2e33;
                color: #f0f0ed;
            }

            QLineEdit:focus, QTextEdit:focus {
                border-color: #6366f1;
            }

            QListWidget[class="file-list"] {
                background: rgba(26, 26, 31, 0.5);
                border: 1px solid #2e2e33;
            }

            QListWidget[class="file-list"]::item:selected {
                background: rgba(99, 102, 241, 0.2);
            }

            QListWidget[class="file-list"]::item:hover {
                background: rgba(99, 102, 241, 0.1);
            }

            QScrollBar::handle:vertical, QScrollBar::handle:horizontal {
                background: #2e2e33;
            }

            QScrollBar::handle:vertical:hover, QScrollBar::handle:horizontal:hover {
                background: #6366f1;
            }
        )";
    } else {
        styleSheet = glassmorphismStyle() + chatStyle() + filePanelStyle() +
                     dashboardStyle() + buttonStyle() + inputStyle() +
                     scrollbarStyle() + tooltipStyle();
    }

    if (qApp) {
        qApp->setStyleSheet(styleSheet);
    }
}

}
