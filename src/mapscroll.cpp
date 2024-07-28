#include <QScrollBar>
#include <QMouseEvent>
#include "mapscroll.h"
#include "mapwidget.h"

CMapScroll::CMapScroll(QWidget *parent)
    : QAbstractScrollArea{parent}
{
    CMapWidget *widget = new CMapWidget(this);
    setViewport(widget);

    // set view attributes
    setAttribute(Qt::WA_MouseTracking);
    setMouseTracking(true);
    setAcceptDrops(true);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setFrameShape(QFrame::NoFrame);
    setAutoFillBackground(false);
    m_mouse.x = m_mouse.y = m_mouse.orgX = m_mouse.orgY = -1;
    m_mouse.lButton = m_mouse.rButton = m_mouse.mButton = false;
    update();
}


void CMapScroll::resizeEvent(QResizeEvent *event)
{
    CMapWidget *glw = dynamic_cast<CMapWidget *>(viewport());
    glw->resizeEvent(event);
    updateScrollbars();
}

void CMapScroll::paintEvent(QPaintEvent *event)
{
    CMapWidget *glw = dynamic_cast<CMapWidget *>(viewport());
    glw->paintEvent(event);
}

void CMapScroll::updateScrollbars()
{
    QSize sz = size();
    int h = sz.width() / FNT_BLOCK_SIZE;
    int v = sz.height() / FNT_BLOCK_SIZE;

    horizontalScrollBar()->setRange(0, m_mapLen - h);
    verticalScrollBar()->setRange(0, m_mapHei - v);

    horizontalScrollBar()->setPageStep(STEPS);
    verticalScrollBar()->setPageStep(STEPS);
}

void CMapScroll::mousePressEvent(QMouseEvent *event)
{
    switch (event->button())
    {
    case Qt::LeftButton:
        m_mouse.lButton = true;
        break;
    case Qt::RightButton:
        m_mouse.rButton = true;
        break;
    default:
        break;
    }


    m_mouse.orgX = m_mouse.x;
    m_mouse.orgY = m_mouse.y;
    setFocus();
    int mx, my;
    topXY(mx, my);
    if (m_mouse.lButton && (m_mouse.x >= 0 && m_mouse.y >= 0))
    {
        CMapWidget *glw = dynamic_cast<CMapWidget *>(viewport());
        int id = glw->at(mx + m_mouse.x, my + m_mouse.y);
        glw->select(id);
        if (id != INVALID) {
            //qDebug("item: %d", id);
        }
        //emit leftClickedAt(m_mouse.x, m_mouse.y);
    }


}

void CMapScroll::mouseReleaseEvent(QMouseEvent *event)
{
    switch (event->button())
    {
    case Qt::LeftButton:
        m_mouse.lButton = false;
        break;
    case Qt::RightButton:
        m_mouse.rButton = false;
        break;
    default:
        break;
    }
    setFocus();
}

void CMapScroll::mouseMoveEvent(QMouseEvent *event)
{
    m_mouse.x = event->pos().x() / SCREEN_PARTION; //+ horizontalScrollBar()->value();
    m_mouse.y = event->pos().y() / SCREEN_PARTION; //+ verticalScrollBar()->value();
    QString str{QString("x: %1 y: %2").arg(m_mouse.x).arg(m_mouse.y)};
    emit statusChanged(str);
    if (m_mouse.lButton && (m_mouse.x >= 0 && m_mouse.y >= 0))
    {
       // emit leftClickedAt(m_mouse.x, m_mouse.y);
    }

    CMapWidget *glw = dynamic_cast<CMapWidget *>(viewport());
    int selected = glw->selected();
    if (m_mouse.lButton && (m_mouse.x >= 0 && m_mouse.y >= 0) && selected != INVALID) {
        int tx =  m_mouse.x - m_mouse.orgX;// : m_mouse.orgX - m_mouse.x;
        int ty = m_mouse.y - m_mouse.orgY;// : m_mouse.orgY - m_mouse.y;
        glw->translate(tx,ty);
    }

    m_mouse.orgX = m_mouse.x;
    m_mouse.orgY = m_mouse.y;

}

void CMapScroll::wheelEvent(QWheelEvent *event)
{
    QPoint numPixels = event->pixelDelta();
    QPoint numDegrees = event->angleDelta() / 8;

    enum
    {
        NONE = 0,
        UP = 1,
        DOWN = 2,
    };
    int dir = NONE;
    if (!numPixels.isNull())
    {
        dir = numPixels.ry() > 0 ? UP : DOWN;
    }
    else
    {
        dir = numDegrees.ry() > 0 ? UP : DOWN;
    }

    int val = verticalScrollBar()->value();
    if (dir == UP)
    {
        val -= STEPS;
        val = std::max(0, val);
    }
    else if (dir == DOWN)
    {
        val += STEPS;
        val = std::min(val, verticalScrollBar()->maximum());
    }
    verticalScrollBar()->setValue(val);
    event->accept();
}


void CMapScroll::newMap(CScript *map)
{
    CMapWidget *glw = dynamic_cast<CMapWidget *>(viewport());
    glw->setMap(map);
    horizontalScrollBar()->setSliderPosition(0);
    verticalScrollBar()->setSliderPosition(0);
    updateScrollbars();
}

void CMapScroll::topXY(int &x, int &y)
{
    x = horizontalScrollBar()->value();
    y = verticalScrollBar()->value();
}
