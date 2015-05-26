#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSocketNotifier>
#include <QKeyEvent>
#include <QDebug>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    con = nullptr;
    //    term = nullptr;
    par.port = 22;
    par.userName = "root";
    par.password = "";
    par.timeout = 5;
    par.authenticationType = QSsh::SshConnectionParameters::AuthenticationTypePassword;
    this->ui->lineip->setText("192.168.11.123");
    ui->textEdit->installEventFilter(this);
}

MainWindow::~MainWindow()
{
    delete ui;
    if (nullptr != con)
    {
        con->deleteLater();
    }

}

void MainWindow::on_pushcon_clicked()
{
    par.host = this->ui->lineip->text();
    con = new QSsh::SshConnection(par);
#if QT_VERSION < 0x050001
    connect(con,SIGNAL(connected()),this,SLOT(sshcon()));
    connect(con,SIGNAL(dataAvailable(const QString)),ui->textEdit, SLOT(append(QString)));
    connect(con,SIGNAL(disconnected()),this, SLOT(disconnected()));
#else
    connect(con,&QSsh::SshConnection::connected,this,&MainWindow::sshcon);
    connect(con,&QSsh::SshConnection::dataAvailable,[this](const QString & mse){this->ui->textEdit->append(mse);});
    connect(con,&QSsh::SshConnection::disconnected,[this](){this->ui->textEdit->append("SSH disconnect");});

#endif
    con->connectToHost();
}

void MainWindow::on_pushsend_clicked()
{
    if (nullptr != con)
    {
        if (con->state() == QSsh::SshConnection::Connected)
        {
            QString str(this->ui->lineEdit->text());
            str += "\r\n";
            m_shell.data()->write(str.toStdString().c_str());
        }
    }
}

void MainWindow::sshcon()
{
    m_shell = con->createRemoteShell();
#if QT_VERSION < 0x050001
    connect(m_shell.data(),SIGNAL(started()),this, SLOT(started()));
    connect(m_shell.data(),SIGNAL(readyReadStandardOutput()),this, SLOT(readAllStandard()));
    connect(m_shell.data(),SIGNAL(readyRead()),this, SLOT(readAllStandard()));
    connect(m_shell.data(),SIGNAL(readyReadStandardError()),this, SLOT(readAll()));
    connect(m_shell.data(),SIGNAL(closed(int)),this, SLOT(stop()));
#else
    connect(m_shell.data(),&QSsh::SshRemoteProcess::started,[this](){
        this->ui->textEdit->insertPlainText("SshRemoteProcess start succsess\n");
        ui->textEdit->moveCursor(QTextCursor::End);
    });
    connect(m_shell.data(),&QSsh::SshRemoteProcess::readyReadStandardOutput,[this](){
        this->ui->textEdit->insertPlainText(QString(this->m_shell.data()->readAllStandardOutput()));
        ui->textEdit->moveCursor(QTextCursor::End);
    });
    connect(m_shell.data(),&QSsh::SshRemoteProcess::readyRead,[this](){
        qDebug() << this->m_shell.data()->readAllStandardOutput();
        this->ui->textEdit->insertPlainText(QString(this->m_shell.data()->readAllStandardOutput()));
        ui->textEdit->moveCursor(QTextCursor::End);
    });
    connect(m_shell.data(),&QSsh::SshRemoteProcess::readyReadStandardError,[this](){
        qDebug() << this->m_shell.data()->readAll();
        this->ui->textEdit->insertPlainText(QString(this->m_shell.data()->readAll()));
        ui->textEdit->moveCursor(QTextCursor::End);
    });
    connect(m_shell.data(),&QSsh::SshRemoteProcess::closed,[this](){
        this->ui->textEdit->insertPlainText("SshRemoteProcess stop\n");
        ui->textEdit->moveCursor(QTextCursor::End);
    });
#endif

    m_shell.data()->start();
}

void MainWindow::started()
{
    ui->textEdit->append(QString("SshRemoteProcess start succsess"));
}

void MainWindow::readAllStandard()
{
    ui->textEdit->append(QString(this->m_shell.data()->readAllStandardOutput()));
}

void MainWindow::readAll()
{
    ui->textEdit->append(QString(this->m_shell.data()->readAll()));
}

void MainWindow::stop()
{
    ui->textEdit->append(QString("SshRemoteProcess stop"));
}

void MainWindow::disconnected()
{
    ui->textEdit->append(QString("SSH disconnect"));
}

bool MainWindow::eventFilter(QObject *obj, QEvent *evt)
{
    if (obj == ui->textEdit) {
        //keyevent
        if (nullptr == con)
        {
            return false;
        }
        if (evt->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(evt);

            if (keyEvent->key() == Qt::Key_Enter
                    || keyEvent->key() == Qt::Key_Return) {
                //m_shell.data()->flush();
                m_shell.data()->write(QString("\r").toStdString().c_str());
                //                stredit = ui->textEdit->toPlainText();
                //                ui->textEdit->setPlainText(stredit.mid(0, stredit.length()-strcmd.length()));
                //                strcmd.clear();
                ui->textEdit->moveCursor(QTextCursor::End);
                return false;
            }//end Enter Return Key;
            switch (keyEvent->key()) {
            case Qt::Key_Backspace: {
                //stredit = ui->textEdit->toPlainText();

                m_shell.data()->write(QString(QChar(0x08)).toLocal8Bit());
                QString s = ui->textEdit->toPlainText();
                s.chop(1);
                ui->textEdit->setPlainText(s);
                ui->textEdit->moveCursor(QTextCursor::End);
                return true;
            }
            case Qt::Key_Up://last cmd CONTROL+P
                m_shell.data()->write(QString(QChar(0x10)).toLocal8Bit());
                return true;
            case Qt::Key_Down://next cms CONTROL+D
                m_shell.data()->write(QString(QChar(0x0E)).toLocal8Bit());
                return true;
            case Qt::Key_Return:
            case Qt::Key_Enter: {
                //m_shell.data()->flush();
                m_shell.data()->write("\r");
                ui->textEdit->moveCursor(QTextCursor::End);
                return true;
            }
            default:
                m_shell.data()->write(keyEvent->text().toLocal8Bit());
                return true;
            }
        }
    }
    return QMainWindow::eventFilter(obj, evt);
}

void MainWindow::on_pushclose_clicked()
{
    if (nullptr != con)
    {
        con->disconnectFromHost();
        con->deleteLater();
        con = nullptr;
    }
}
