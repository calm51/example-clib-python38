#pragma once
#pragma execution_character_set("utf-8")

#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QPair>

#if defined Q_OS_ANDROID
#include "QtAndroid"
#include <QtAndroidExtras>
#include <QAndroidJniObject>
#include <QAndroidJniEnvironment>
#include <qandroidjniobject.h>

#include <qtandroidcls/qtandroidcls.h>
#endif

#include <qmessagebox.h>

#pragma push_macro("slots")
#undef slots
#include "Python.h"
#pragma pop_macro("slots")


#if defined Q_OS_ANDROID
#include "qtpyandassets/pyqtdeploy_start.h"
#include <android/log.h>
#include "gdbm.h"
#endif

#include <QIODevice>
#include <QFileDevice>
#include <QTemporaryFile>
#include <QTextStream>
#include <unistd.h>
#include <fcntl.h>
#include <string>
#include <unistd.h>
#include <iostream>
#include <qdir.h>
#include <QSocketNotifier>

bool copyDir(const QString &srcPath, const QString &dstPath){
    QDir srcDir(srcPath); QDir dstDir(dstPath);
    if (!srcDir.exists()) {qDebug() << "Source directory does not exist."; return true;}
    if (!dstDir.exists()) {if (!dstDir.mkpath(dstPath)) {qDebug() << "Failed to create target directory.";return true;}}
    QFileInfoList fileInfoList = srcDir.entryInfoList(QDir::Dirs | QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot);
    for (int i = 0; i < fileInfoList.size(); i++) {
        QFileInfo fileInfo = fileInfoList.at(i);
        QString srcFilePath = fileInfo.absoluteFilePath();
        QString dstFilePath = dstPath + QDir::separator() + fileInfo.fileName();
        if (fileInfo.isDir()) {copyDir(srcFilePath, dstFilePath);
        }else { // ____<<"cp: "<<srcFilePath<< dstFilePath;
            QFile::copy(srcFilePath, dstFilePath);
        }
    }
    return true;
}

#ifdef Q_OS_ANDROID
QString setLD_LIBRARY_PATH() {
    QAndroidJniObject activity = QtAndroid::androidActivity();
    if (activity.isValid()) {
        QAndroidJniObject context = activity.callObjectMethod("getApplicationContext", "()Landroid/content/Context;");
        if (context.isValid()) {
            QAndroidJniObject filesDir = context.callObjectMethod("getFilesDir", "()Ljava/io/File;");
            if (filesDir.isValid()) {
                QString filesPath = filesDir.callObjectMethod("getAbsolutePath", "()Ljava/lang/String;").toString();
                QString ldLibraryPath = qgetenv("LD_LIBRARY_PATH");
                if (!ldLibraryPath.isEmpty()) {ldLibraryPath += ":";}
                ldLibraryPath += filesPath;
                setenv("LD_LIBRARY_PATH", ldLibraryPath.toUtf8().constData(), 1);
                qDebug() << "LD_LIBRARY_PATH set to:" << qgetenv("LD_LIBRARY_PATH");
                return filesPath;
            }
        }
    }
    return QString();
}
#endif


int main(int argc, char *argv[]){
    //    qDebug()<<QByteArray(argv[0]);
    //    QByteArray byteArray;
    //    for (int i = 0; i < argc; ++i) { byteArray.append(argv[i]); byteArray.append(' '); }
    //    // byteArray.chop(1);
    //    qDebug()<<byteArray;

    //    qDebug()<<gdbm_version_number;

    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication a(argc, argv);
    a.setAttribute(Qt::ApplicationAttribute::AA_UseHighDpiPixmaps);
    a.setQuitOnLastWindowClosed(false);
    a.setStyle("Fusion");
#if defined Q_OS_ANDROID
    QtAndroidCls *qac = QtAndroidCls::instance("ca/calm/pythondemo/pythondemo");
#endif
    a.setWindowIcon(QIcon(":/resource/icon/main.ico"));

    MainWindow w;
    w.setWindowTitle("pythondemo");
#ifdef Q_OS_ANDROID
    w.show();
    QTimer::singleShot(50,qac,[&](){
        qac->to_statusbar_text_white();
        QtAndroid::hideSplashScreen(100);
        qac->to_statusbar_text_black();
    });
    qac->up_statusbar_height();
#endif
    w.show();
#ifdef Q_OS_ANDROID
    //    QString AppDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    //    QAndroidJniObject androidContext = QtAndroid::androidContext();
    //    QAndroidJniObject dir = QAndroidJniObject::fromString(QString(""));
    //    QAndroidJniObject path = androidContext.callObjectMethod("getExternalFilesDir","(Ljava/lang/String;)Ljava/io/File;", dir.object());
    //    //    AppDataDir = path.toString();
    //    qDebug()<<AppDataDir;
    //    QDir d1(QDir("assets:").absoluteFilePath("python3.8-py"));
    //    QDir d2(QDir(AppDataDir).absoluteFilePath("python3.8-py"));
    //    //    qDebug()<<QDir(d1).entryList()<<"\n"<<QDir(d2).entryList();
    //    if (d1.exists() && !d2.exists()){
    //        d2.mkpath(d2.absolutePath()); copyDir(d1.absolutePath(),d2.absolutePath());
    //        // qDebug()<<QDir(d2).entryList();
    //    }
#else
    QString AppDataDir = qApp->applicationDirPath();
#endif

    QString libPathStrings = "[";
    QStringList libPaths = QCoreApplication::libraryPaths();
    // const QString &filesPath = setLD_LIBRARY_PATH();
    // if (filesPath.count()>0){ libPaths.append(filesPath);}
    foreach (const QString &path, libPaths) {libPathStrings.append(QString("\"")+path+QString("\","));}libPathStrings.append("]");
#ifdef Q_OS_ANDROID
    qDebug()<<"pyqtdeploy_start: "<<pyqtdeploy_start(argc, argv, extension_modules, "__main__", NULL, NULL);
#else
    QStringList pLibDirs({
                         #ifdef Q_OS_ANDROID
                             QDir(QDir(AppDataDir).absoluteFilePath("python3.8-py")).absoluteFilePath("base_library.zip"),
                             QDir(AppDataDir).absoluteFilePath("python3.8-py"),
                             QDir(QDir(AppDataDir).absoluteFilePath("python3.8-py")).absoluteFilePath("lib-dynload"),
                             QDir(QDir(AppDataDir).absoluteFilePath("python3.8-py")).absoluteFilePath("site-packages"),
                         #elif defined(Q_OS_LINUX) || defined(Q_OS_WIN)
                             QDir(QDir(AppDataDir).absoluteFilePath("python3.8-pyc")).absoluteFilePath("base_library.zip"),
                             QDir(AppDataDir).absoluteFilePath("python3.8-pyc"),
                             QDir(QDir(AppDataDir).absoluteFilePath("python3.8-pyc")).absoluteFilePath("lib-dynload"),
                             QDir(QDir(AppDataDir).absoluteFilePath("python3.8-pyc")).absoluteFilePath("site-packages"),
                         #endif

                         });
    Py_SetPath(pLibDirs.join(":").toStdWString().c_str());
#endif
    Py_Initialize();

    //    _PyCoreConfig config = _PyCoreConfig_INIT;
    //    _PyInitError status = _Py_InitializeFromConfig(&config);
    //    qDebug()<<status.user_err<<QString(QByteArray(status.msg))<<QString(QByteArray(status.prefix));
    //    //    if (status != PY_SUCCESS) {
    //    //        fprintf(stderr, "Failed to initialize Python interpreter\n");
    //    //        qDebug()<<22222222222222;
    //    //        return 1;
    //    //    }

    //    PyStatus status;
    //    PyConfig config;
    //    PyConfig_InitPythonConfig(&config);
    //    //    config.isolated = 1;
    //    status = PyConfig_SetBytesArgv(&config, argc, argv);
    //    if (PyStatus_Exception(status)) {qDebug()<<"PyStatus_Exception(status)"<<QString(QByteArray(status.err_msg));}
    //    status = Py_InitializeFromConfig(&config);
    //    if (PyStatus_Exception(status)) {qDebug()<<"PyStatus_Exception(status)"<<QString(QByteArray(status.err_msg));}
    //    PyObject *ptype, *pvalue, *ptraceback;
    //    PyErr_Fetch(&ptype, &pvalue, &ptraceback);
    //    PyErr_NormalizeException(&ptype, &pvalue, &ptraceback);
    //    PyObject *pStrErrorMessage = PyObject_Str(pvalue);
    //    const char *errorMessage = PyUnicode_AsUTF8(pStrErrorMessage);
    //    qDebug()<<errorMessage;
    //    //    Py_DECREF(ptype);
    //    //    Py_DECREF(pvalue);
    //    //    Py_XDECREF(ptraceback);
    //    //    Py_DECREF(pStrErrorMessage);
    //    qDebug()<<1111111111111;
    //    return a.exec();

    const char *_python_fds_prefix = "[python]";
    int fds[2];
    if (pipe(fds) == -1) {qDebug() <<_python_fds_prefix<< "failed to create pipe.";}
    else{
        int flags = fcntl(fds[0], F_GETFL, 0);
        fcntl(fds[0], F_SETFL, flags | O_NONBLOCK);
        qDebug()<<_python_fds_prefix<<"new fds"<<fds[0]<<fds[1];
        QSocketNotifier *notifier = new QSocketNotifier(fds[0], QSocketNotifier::Read);
        QObject::connect(notifier, &QSocketNotifier::activated, [=]() {
            ssize_t rdsz;
            char buf[10240];
            while((rdsz = read(fds[0], buf, sizeof buf - 1)) > 0) {
                if(buf[rdsz - 1] == '\n') --rdsz;
                buf[rdsz] = 0;
#ifdef Q_OS_ANDROID
                __android_log_write(ANDROID_LOG_DEBUG, "python", buf);
#else
                qDebug()<<_python_fds_prefix<<buf;
#endif
            }
            //            char buffer[1024];ssize_t bytesRead;
            //            // qDebug()<< _python_fds_prefix; //  qDebug().noquote().nospace() // << " "
            //            while ((bytesRead = read(fds[0], buffer, sizeof(buffer))) > 0) {
            //                qDebug() << _python_fds_prefix<<QString::fromUtf8(QByteArray(buffer, bytesRead)).toUtf8().constData();
            //            } ;
            //            if (bytesRead == -1 && errno != EAGAIN) {qDebug() << _python_fds_prefix<<"failed to read from pipe.";}
        });
        const QString&changeFileId = QString(R"(
try:
    import sys, os
    sys.path.extend(%1)
    existing_fd = %2
    if sys.stdout.fileno() != existing_fd:
        sys.stdout = sys.stderr = os.fdopen(existing_fd, 'w', encoding="utf-8", buffering=1, closefd=False)
    _qt_LD_LIBRARY_PATH = "%3:"
    if len(_qt_LD_LIBRARY_PATH)>1:
        os.environ["PATH"] = _qt_LD_LIBRARY_PATH+os.environ["PATH"]
        os.environ["LD_LIBRARY_PATH"] = _qt_LD_LIBRARY_PATH+os.environ["LD_LIBRARY_PATH"]
    #print(sys.path)
    #print(sys.path_hooks)
    #print(os.environ)
except Exception as e:
    import traceback
    print("".join(traceback.TracebackException.from_exception(e).format()))
    #raise IOError(str(e))
)").arg(libPathStrings).arg(QString::number(fds[1])).arg(QString(qgetenv("LD_LIBRARY_PATH")));
        qDebug()<<_python_fds_prefix<<"change fileno result:"<<PyRun_SimpleString(changeFileId.toUtf8().constData());
    }

    const char* code = R"(
try:
    import ssl
    openssl_version = ssl.OPENSSL_VERSION
    print("OpenSSL is loaded. Version:", openssl_version)
except Exception as e:
    import traceback
    print("".join(traceback.TracebackException.from_exception(e).format()))
)";
    if (PyRun_SimpleString(code) != 0){
        //        qDebug()<<_python_fds_prefix<< getPythonErrorString();
        //            PyObject *pType, *pValue, *pTraceback;
        //            PyErr_Fetch(&pType, &pValue, &pTraceback);
        //            PyErr_NormalizeException(&pType, &pValue, &pTraceback);
        //            // Get exception type
        //            PyObject *pTypeName = PyObject_GetAttrString(pType, "__name__");
        //            const char *typeName = PyUnicode_AsUTF8(pTypeName);
        //            // Get exception message
        //            const char *errorMessage = PyUnicode_AsUTF8(PyObject_Str(pValue));
        //            // Print exception info
        //            printf("Exception Type: %s\n", typeName);
        //            printf("Exception Message: %s\n", errorMessage);
        //            // Clean up
        //            Py_DECREF(pType);
        //            Py_DECREF(pValue);
        //            Py_XDECREF(pTraceback);
        //            Py_XDECREF(pTypeName);
    }

    //    PyObject *sysModule = PyImport_ImportModule("sys");
    //    if (sysModule) {
    //        PyObject *pythonFd = PyLong_FromLong(fds[1]);
    //        if (pythonFd) {
    //            PyObject_SetAttrString(sysModule, "stdout", pythonFd);
    //            Py_DECREF(pythonFd);
    //        } else {
    //            qDebug() << "Failed to create Python file descriptor.";
    //        }
    //        Py_DECREF(sysModule);
    //    } else {
    //        qDebug() << "Failed to import Python sys module.";
    //    }


    //    Py_Finalize();

    return a.exec();

}

