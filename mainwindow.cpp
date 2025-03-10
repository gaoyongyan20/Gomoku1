#include <QPainter>
#include <QTimer>
#include <QSound>
#include <QString>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QMouseEvent>
#include <QMessageBox>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QDebug>
#include <math.h>
#include "mainwindow.h"

// -------全局遍历-------//
#define CHESS_ONE_SOUND "/opt/sound/chessone.wav"
#define WIN_SOUND "/opt/sound/win.wav"
#define LOSE_SOUND "/opt/sound/lose.wav"
#define BGM "/opt/sound/bgm.wav"


const int kBoardMargin = 30; // 棋盘边缘空隙
const int kRadius = 15; // 棋子半径
const int kMarkSize = 6; // 落子标记边长
const int kBlockSize = 40; // 格子的大小
const int kPosDelta = 20; // 鼠标点击的模糊距离上限

const int kAIDelay = 700; // AI下棋的思考时间
const int resetDelay = 3000;

// -------------------- //

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // 设置棋盘大小
    setFixedSize(kBoardMargin * 2 + kBlockSize * kBoardSizeNum, kBoardMargin * 2 + kBlockSize * (kBoardSizeNum + 5));

    // 开启鼠标hover功能，这两句一般要设置window的
    setMouseTracking(true);
    //    centralWidget()->setMouseTracking(true);

    // 添加菜单
    QMenu *gameMenu = menuBar()->addMenu(tr("VS Mode")); // menuBar默认是存在的，直接加菜单就可以了
    QAction *actionPVP = new QAction("Person VS Person", this);
    connect(actionPVP, SIGNAL(triggered()), this, SLOT(initPVPGame()));
    gameMenu->addAction(actionPVP);


    QAction *actionPVE = new QAction("Person VS Computer", this);
    connect(actionPVE, SIGNAL(triggered()), this, SLOT(initPVEGame()));
    gameMenu->addAction(actionPVE);



    actionMeg = new QAction("who will win?", this);
    actionMeg->setEnabled(false);
    connect(this, SIGNAL(whoWinMsgChanged()), this, SLOT(controlMsg()));
    menuBar()->addAction(actionMeg);



    QAction *actionRestart = new QAction("restart", this);
    connect(actionRestart,SIGNAL(triggered()), this, SLOT(controlRestart()));
    menuBar()->addAction(actionRestart);


    QAction *actionMusic = new QAction("♫ ♫", this);
    connect(actionMusic,SIGNAL(triggered()), this, SLOT(controlBgm()));
    menuBar()->addAction(actionMusic);


    // 设置落子声音和背景音乐的播放器
    player1 = new QMediaPlayer;
    player2 = new QMediaPlayer;

    player1->setMedia(QUrl::fromLocalFile(CHESS_ONE_SOUND));
    player2->setMedia(QUrl::fromLocalFile(BGM));

    player1->setVolume(20);
    player2->setVolume(30);

    // 控制背景音乐循环
    connect(player2, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(controlLoopBgm()));
    player2->play();

    // 设置背景颜色
    setStyleSheet("background-color: #d2691e;");
    // 开始游戏
    initGame();
}

MainWindow::~MainWindow()
{
    if (game)
    {
        delete game;
        game = nullptr;
    }
}

void MainWindow::initGame()
{
    // 初始化游戏模型
    game = new GameModel;
    initPVPGame();
}

void MainWindow::initPVPGame()
{
    game_type = PERSON;
    game->gameStatus = PLAYING;
    game->startGame(game_type);
    update();
}

void MainWindow::initPVEGame()
{
    game_type = BOT;
    game->gameStatus = PLAYING;
    game->startGame(game_type);
    update();
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    // 绘制棋盘
    painter.setRenderHint(QPainter::Antialiasing, true); // 抗锯齿

    for (int i = 0; i < kBoardSizeNum + 1; i++)
    {
        painter.drawLine(kBoardMargin + kBlockSize * i, kBoardMargin, kBoardMargin + kBlockSize * i, size().height() - kBoardMargin);
        painter.drawLine(kBoardMargin, kBoardMargin + kBlockSize * i, size().width() - kBoardMargin, kBoardMargin + kBlockSize * i);
    }

    QBrush brush;
    brush.setStyle(Qt::SolidPattern);
    // 绘制落子标记(防止鼠标出框越界)
    if (clickPosRow > 0 && clickPosRow < kBoardSizeNum &&
        clickPosCol > 0 && clickPosCol < kBoardSizeNum &&
        game->gameMapVec[clickPosRow][clickPosCol] == 0)
    {
        if (game->playerFlag)
            brush.setColor(Qt::white);
        else
            brush.setColor(Qt::black);
        painter.setBrush(brush);
        painter.drawRect(kBoardMargin + kBlockSize * clickPosCol - kMarkSize / 2, kBoardMargin + kBlockSize * clickPosRow - kMarkSize / 2, kMarkSize, kMarkSize);
    }

    // 绘制棋子
    for (int i = 0; i < kBoardSizeNum; i++)
        for (int j = 0; j < kBoardSizeNum; j++)
        {
            if (game->gameMapVec[i][j] == 1)
            {
                brush.setColor(Qt::white);
                painter.setBrush(brush);
                painter.drawEllipse(kBoardMargin + kBlockSize * j - kRadius, kBoardMargin + kBlockSize * i - kRadius, kRadius * 2, kRadius * 2);
            }
            else if (game->gameMapVec[i][j] == -1)
            {
                brush.setColor(Qt::black);
                painter.setBrush(brush);
                painter.drawEllipse(kBoardMargin + kBlockSize * j - kRadius, kBoardMargin + kBlockSize * i - kRadius, kRadius * 2, kRadius * 2);
            }
        }

    // 判断输赢
    if (clickPosRow > 0 && clickPosRow < kBoardSizeNum &&
        clickPosCol > 0 && clickPosCol < kBoardSizeNum &&
        (game->gameMapVec[clickPosRow][clickPosCol] == 1 ||
         game->gameMapVec[clickPosRow][clickPosCol] == -1))
    {

        if (game->isWin(clickPosRow, clickPosCol) && game->gameStatus == PLAYING)
        {

            if (game->gameMapVec[clickPosRow][clickPosCol] == 1)
                whoWinMsg = "- white  win -";
            else if (game->gameMapVec[clickPosRow][clickPosCol] == -1)
                whoWinMsg = "- black  win -";
            emit whoWinMsgChanged();

            player1->setMedia(QUrl::fromLocalFile(WIN_SOUND));
            player1->play();

            game->gameStatus = WIN;

            game->startGame(game_type);
            game->gameStatus = PLAYING;

            // 设置重置游戏的定时
            QTimer::singleShot(resetDelay, this, SLOT(update()));


        }
    }


    // 判断死局
    if (game->isDeadGame())
    {

        QMessageBox::StandardButton btnValue = QMessageBox::information(this, "oops", "dead game!");
        if (btnValue == QMessageBox::Ok)
        {
            player1->setMedia(QUrl::fromLocalFile(LOSE_SOUND));
            player1->play();
            game->startGame(game_type);
            game->gameStatus = PLAYING;
        }

    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    // 通过鼠标的hover确定落子的标记
    int x = event->x();
    int y = event->y();

    // 棋盘边缘不能落子
    if (x >= kBoardMargin + kBlockSize / 2 &&
        x < size().width() - kBoardMargin &&
        y >= kBoardMargin + kBlockSize / 2 &&
        y < size().height()- kBoardMargin)
    {
        // 获取最近的左上角的点
        int col = x / kBlockSize;
        int row = y / kBlockSize;

        int leftTopPosX = kBoardMargin + kBlockSize * col;
        int leftTopPosY = kBoardMargin + kBlockSize * row;

        // 根据距离算出合适的点击位置,一共四个点，根据半径距离选最近的
        clickPosRow = -1; // 初始化最终的值
        clickPosCol = -1;
        int len = 0; // 计算完后取整就可以了

        // 确定一个误差在范围内的点，且只可能确定一个出来
        len = sqrt((x - leftTopPosX) * (x - leftTopPosX) + (y - leftTopPosY) * (y - leftTopPosY));
        if (len < kPosDelta)
        {
            clickPosRow = row;
            clickPosCol = col;
        }
        len = sqrt((x - leftTopPosX - kBlockSize) * (x - leftTopPosX - kBlockSize) + (y - leftTopPosY) * (y - leftTopPosY));
        if (len < kPosDelta)
        {
            clickPosRow = row;
            clickPosCol = col + 1;
        }
        len = sqrt((x - leftTopPosX) * (x - leftTopPosX) + (y - leftTopPosY - kBlockSize) * (y - leftTopPosY - kBlockSize));
        if (len < kPosDelta)
        {
            clickPosRow = row + 1;
            clickPosCol = col;
        }
        len = sqrt((x - leftTopPosX - kBlockSize) * (x - leftTopPosX - kBlockSize) + (y - leftTopPosY - kBlockSize) * (y - leftTopPosY - kBlockSize));
        if (len < kPosDelta)
        {
            clickPosRow = row + 1;
            clickPosCol = col + 1;
        }
    }

    // 存了坐标后也要重绘
    update();
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    // 人下棋，并且不能抢机器的棋
    if (!(game_type == BOT && !game->playerFlag))
    {
        chessOneByPerson();
        // 如果是人机模式，需要调用AI下棋
        if (game->gameType == BOT && !game->playerFlag)
        {
            // 用定时器做一个延迟
            QTimer::singleShot(kAIDelay, this, SLOT(chessOneByAI()));
        }
    }

}

void MainWindow::chessOneByPerson()
{
    // 根据当前存储的坐标下子
    // 只有有效点击才下子，并且该处没有子
    if (clickPosRow != -1 && clickPosCol != -1 && game->gameMapVec[clickPosRow][clickPosCol] == 0)
    {
        player1->setMedia(QUrl::fromLocalFile(CHESS_ONE_SOUND));
        player1->play();
        game->actionByPerson(clickPosRow, clickPosCol);
        // 重绘
        update();
    }
}

void MainWindow::chessOneByAI()
{
    game->actionByAI(clickPosRow, clickPosCol);
    player1->setMedia(QUrl::fromLocalFile(CHESS_ONE_SOUND));
    player1->play();
    update();
}

void MainWindow::controlBgm()
{
    if(player2->state() == QMediaPlayer::PlayingState)
    {
        player2->pause();
    }
    else if(player2->state() == QMediaPlayer::PausedState)
    {
        player2->play();

    }
}

void MainWindow::controlRestart()
{
    //
    game->startGame(game_type);
    game->gameStatus = PLAYING;

    //
    update();
}

void MainWindow::controlLoopBgm()
{
    if(player2->state() == QMediaPlayer::StoppedState)
    {
        player2->setMedia(QUrl::fromLocalFile(BGM));
        player2->play();
        qDebug() << "nihao";
    }
    return;
}

void MainWindow::controlMsg()
{
    actionMeg->setText(whoWinMsg);
    // 当时间到的时候，who win信息改为默认信息
    QTimer::singleShot(resetDelay, this, SLOT(resetMsg()));

}

void MainWindow::resetMsg()
{
    actionMeg->setText("who will win?");
}
