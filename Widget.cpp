#include "Widget.h"

#include <QApplication>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>

#include "XAudioSound.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , m_pSoundPlay(new XAudioSound())
{
    m_pMainLayout = new QVBoxLayout;

    m_pInputEdit = new QLineEdit("", this);
    m_pSearchButton = new QPushButton("browser", this);
    connect(m_pSearchButton, &QPushButton::clicked,
            this, &Widget::onBrowserButtonClicked);

    auto pLabel = new QLabel("audioFile:", this);
    auto pHLayout = new QHBoxLayout;
    pHLayout->addWidget(pLabel);
    pHLayout->addWidget(m_pInputEdit);
    pHLayout->addWidget(m_pSearchButton);

    m_pPlayButton = new QPushButton("play", this);
    m_pPlayButton->setEnabled(false);
    connect(m_pPlayButton, &QPushButton::clicked,
            this, &Widget::onPlayButtonClicked);

    pHLayout->addSpacing(20);
    pHLayout->addWidget(m_pPlayButton);
    m_pMainLayout->addLayout(pHLayout);

    m_pMainLayout->addStretch();
    setLayout(m_pMainLayout);
}

Widget::~Widget()
{
    delete m_pSoundPlay;
    m_pSoundPlay = nullptr;
}

void Widget::onBrowserButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open Audio File",
                                                     "/home",
                                                     "Audio (*.wav *.pcm)");

    m_pInputEdit->setText(fileName);
    m_pPlayButton->setEnabled(!fileName.isEmpty());
}

void Widget::onPlayButtonClicked()
{
    QString audioFile = m_pInputEdit->text();
    bool bResult = m_pSoundPlay->play(audioFile);

    qDebug() << "play result:" << bResult;
}

