#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <sshconnection.h>
#include <qsharedpointer.h>
#include <sshremoteprocess.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
protected:
    bool eventFilter(QObject *, QEvent *);
public slots:
    void on_pushcon_clicked();
    void on_pushsend_clicked();
    void sshcon();

    void started();
    void readAllStandard();
    void readAll();
    void stop();
    void disconnected();

private slots:
    void on_pushclose_clicked();

private:
    Ui::MainWindow *ui;
    QSsh::SshConnectionParameters par;
    QSsh::SshConnection * con;
    QSharedPointer<QSsh::SshRemoteProcess> m_shell;

};

#endif // MAINWINDOW_H
