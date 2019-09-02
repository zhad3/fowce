#include <fstream>
#include <iostream>

#include <QApplication>
#include <QSettings>
#include <QLocale>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QPlainTextEdit>

#include <QDebug>

#include <yaml-cpp/yaml.h>

#include "mainwindow.h"
#include "util.h"

static QString LOG_DIR;

void FMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    const char *file = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";

    QString time = QDateTime::currentDateTime().toString("[yyyy-MM-dd hh:mm:ss]");
    QString prefix = "[Debug]";
    switch(type) {
    case QtDebugMsg:    break;
    case QtInfoMsg:     prefix = "[Info]"; break;
    case QtWarningMsg:  prefix = "[Warning]"; break;
    case QtCriticalMsg: prefix = "[Critical]"; break;
    case QtFatalMsg:    prefix = "[Fatal]"; break;
    }

    QString out_msg = prefix + time + QString(" %1 (Line: %2, Function: %3, File: %4)\n").arg(msg, QString(context.line), QString(function), QString(file));
    QByteArray filename = QDir(LOG_DIR).filePath(QDateTime::currentDateTime().toString("yyyyMMdd") + ".log").toUtf8();
    std::ofstream out(filename.data(), std::ofstream::app);
    out << out_msg.toUtf8().data();
    out.close();
    std::cerr << out_msg.toUtf8().data();
}

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName(ORGNAME);
    QCoreApplication::setApplicationName(APPNAME);

#ifndef QT_DEBUG
    QDir logDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    logDir.mkdir("logs");
    LOG_DIR = logDir.filePath("logs");
    qInstallMessageHandler(FMessageOutput);
#endif

    QApplication a(argc, argv);
    QSettings::setDefaultFormat(QSettings::IniFormat);

    QSettings settings;
    QLocale locale = QLocale(settings.value("main/locale", "en_US").toString());
    QLocale::setDefault(locale);

    MainWindow w;
    w.show();

    return a.exec();
}
