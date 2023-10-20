/*
Copyright (C) 2023  Quectel Wireless Solutions Co.,Ltd.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#include "mainwindow.h"
#include "ccrashstack.h"
#include "QDir"
#include "LogModule/openlog.h"

#include <QApplication>
#include <QStandardPaths>
#include <QSystemSemaphore>
#include <QSharedMemory>
#include <QMessageBox>
#include <QMutex>
#include <QDateTime>
#include <QTranslator>

QTranslator *main_translator = NULL;

int main(int argc, char *argv[])
{
    //日志模块，开始日志储存
    OpenLog *openlog = new OpenLog();
    openlog->setLog();

    QApplication a(argc, argv);
    QSystemSemaphore semaphore("SingleAppTest2Semaphore", 1);
    semaphore.acquire();

#ifndef Q_OS_WIN32
    // 在linux / unix 程序异常结束共享内存不会回收
    // 在这里需要提供释放内存的接口，就是在程序运行的时候如果有这段内存 先清除掉
    QSharedMemory nix_fix_shared_memory("SingleAppTest2");
    if (nix_fix_shared_memory.attach())
    {undefined
        nix_fix_shared_memory.detach();
    }
#endif

    QSharedMemory sharedMemory("QthTools-Auth-Burner");
    bool isRunning = false;
    if (sharedMemory.attach())
    {
        isRunning = true;
    }
    else
    {
        sharedMemory.create(1);
        isRunning = false;
    }
    semaphore.release();

    // 如果您已经运行了应用程序的一个实例，那么我们将通知用户
    if (isRunning)
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("错误");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("The application is already running.\n"
            "Allowed to run only one instance of the application.");
        msgBox.exec();
        return 1;
    }

    QLocale locale;
    if(NULL != main_translator)
    {
        qApp->removeTranslator(main_translator);
        delete main_translator;
        main_translator = NULL;

    }
    main_translator = new QTranslator();

    if(locale.language() == QLocale::Chinese)
    {
        main_translator->load(":/chinese.qm");
        qApp->installTranslator(main_translator);
    }
    else
    {
        main_translator->load(":/english.qm");
        qApp->installTranslator(main_translator);
    }

    MainWindow w;
    w.show();
    return a.exec();
}
