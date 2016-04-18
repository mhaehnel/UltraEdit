#pragma once
#include <QVideoWidget>

class VideoWidget : public QVideoWidget
{
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = 0);
    void mouseDoubleClickEvent(QMouseEvent*);

signals:

public slots:

};
