#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QPlainTextEdit>
#include <QDir>
#include <QDateTime>
#include <QStandardPaths>
#include <QTextStream>

class Logger : QObject
{
    Q_OBJECT
public:
    explicit Logger();
    explicit Logger(QObject *parent, QPlainTextEdit *editor = nullptr);
    ~Logger();
    void setEditor(QPlainTextEdit *editor);

private:
    QFile *file;
    QString m_errorFileName;
    QDir *m_logsDir;
    QPlainTextEdit *m_editor;

    bool openFile();

public slots:
    void Error(const QString &message);
};

#endif // LOGGER_H
