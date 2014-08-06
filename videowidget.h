#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

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

#endif // VIDEOWIDGET_H
