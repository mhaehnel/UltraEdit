#include "videowidget.h"

VideoWidget::VideoWidget(QWidget* parent) :
    QVideoWidget(parent)
{
}

void VideoWidget::mouseDoubleClickEvent(QMouseEvent *) {
    setFullScreen(!isFullScreen());
}
