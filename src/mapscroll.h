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
    void newMap(CScript *map);

protected:

    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void wheelEvent(QWheelEvent *) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    void updateScrollbars();
    void topXY(int &x, int &y);

    using Mouse = struct
    {
        int orgX;
        int orgY;
        int x;
        int y;
        bool lButton;
        bool rButton;
        bool mButton;
    };
    Mouse m_mouse;

    int m_mapLen = 256;
    int m_mapHei = 256;
    enum
    {
        FNT_BLOCK_SIZE = 8,
        SCREEN_PARTION = 2 * FNT_BLOCK_SIZE,
        GRID_SIZE = 32,
        STEPS = 4,
        INVALID = -1,
    };
};

#endif // CMAPSCROLL_H
