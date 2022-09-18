#pragma once

#include <QWidget>

#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QProgressBar>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>

class XAudioSound;

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private Q_SLOTS:
    void onBrowserButtonClicked();
    void onPlayButtonClicked();

private:
    XAudioSound     *m_pSoundPlay = nullptr;

    QVBoxLayout     *m_pMainLayout = nullptr;
    QLineEdit       *m_pInputEdit = nullptr;
    QPushButton     *m_pSearchButton = nullptr;
    QPushButton     *m_pPlayButton = nullptr;
};
