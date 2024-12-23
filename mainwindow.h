// 游戏的主窗口类，主要设计游戏界面
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include "GameModel.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    // 绘制
    void paintEvent(QPaintEvent *event);
    // 监听鼠标移动情况，方便落子
    void mouseMoveEvent(QMouseEvent *event);
    // 实际落子
    void mouseReleaseEvent(QMouseEvent *event);

signals:
    void whoWinMsgChanged();
private:
    GameModel *game; // 游戏指针
    GameType game_type; // 存储游戏类型
    int clickPosRow, clickPosCol; // 存储将点击的位置
    QMediaPlayer *player1;//
    QMediaPlayer *player2;//
    QString whoWinMsg;//
    QAction *actionMeg;
    void initGame();
    void checkGame(int y, int x);

private slots:
    void chessOneByPerson(); // 人执行
    void chessOneByAI(); // AI下棋

    void initPVPGame();
    void initPVEGame();

    void controlBgm(); // 控制背景音乐
    void controlRestart(); // 重置游戏
    void controlLoopBgm(); // 控制背景音乐循环播放
    void controlMsg(); // 显示who win
    void resetMsg(); // 重置who win
};

#endif // MAINWINDOW_H
