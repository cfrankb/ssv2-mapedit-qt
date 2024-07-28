#ifndef CMAPSCROLL_H
#define CMAPSCROLL_H

#include <QAbstractScrollArea>
#include <QObject>

class CScript;

class CMapScroll : public QAbstractScrollArea
{
    Q_OBJECT
public:
    explicit CMapScroll(QWidget *parent = nullptr);

signals:
    void statusChanged(const QString str);
    void leftClickedAt(int x, int y);

protected slots:
    void newMapSize(int len, int hei);
    void newMap(CScript *map);

protected:

    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void wheelEvent(QWheelEvent *) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    void updateScrollbars();

    typedef struct
    {
        int x;
        int y;
        bool lButton;
        bool rButton;
        bool mButton;
    } Mouse;
    Mouse m_mouse;

    int m_mapLen = 256;
    int m_mapHei = 256;
    enum
    {
        FNT_BLOCK_SIZE = 8,
        GRID_SIZE = 32,
        STEPS = 4
    };
};

#endif // CMAPSCROLL_H
