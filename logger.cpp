#include <QDateTime>
#include <QStandardPaths>
#include <QTextStream>

#include "logger.h"

Logger::Logger()
{
    file = new QFile();
    QString path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QDir *dir = new QDir(path);
    dir->mkdir("logs");
    dir->cd("logs");
    m_logsDir = dir;
}

Logger::Logger(QObject *parent, QPlainTextEdit *editor) :
    QObject(parent), m_editor(editor)
{
    file = new QFile();
    QString path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    QDir *dir = new QDir(path);
    dir->mkdir("logs");
    dir->cd("logs");
    m_logsDir = dir;
}

Logger::~Logger()
{
    if (file->isOpen()) {
        file->close();
    }
}

void Logger::setEditor(QPlainTextEdit *editor)
{
    m_editor = editor;
}

bool Logger::openFile()
{
    if (!file->isOpen()) {
        QString today = QDateTime::currentDateTime().toString("yyyyMMdd");
        QString errorFile = QString("error.") + today + QString(".log");
        file->setFileName(m_logsDir->filePath(errorFile));
        return file->open(QIODevice::Append | QIODevice::Text);
    }
    return true;
}

void Logger::Error(const QString &message)
{
    QString text = message;
    text = QString("[Error] ") + QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss] ") + text;
    if (openFile()) {
        QTextStream out(file);
        out.setCodec("UTF-8");
        out << text;
    }

    if (m_editor) {
        m_editor->appendPlainText(text);
    }
}
