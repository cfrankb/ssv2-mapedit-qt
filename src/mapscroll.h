#ifndef CMAPSCROLL_H
#define CMAPSCROLL_H

#include <unordered_map>
#include <QAbstractScrollArea>
#include <QObject>

class CScript;

class CMapScroll : public QAbstractScrollArea
{
    Q_OBJECT
public:
    explicit CMapScroll(QWidget *parent = nullptr);
    int topX();
    int topY();

signals:
    void selectionChanged();
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
    virtual void keyPressEvent(QKeyEvent* event) override;
    virtual void keyReleaseEvent(QKeyEvent* event) override;
    void updateScrollbars();
    void topXY(int &x, int &y);
    void keyReflector(uint32_t key, uint8_t state);

    using mouse_t = struct
    {
        int orgX;
        int orgY;
        int x;
        int y;
        bool lButton;
        bool rButton;
        bool mButton;
    };
    mouse_t m_mouse;

    enum
    {
        KEY_RELEASED = 0,
        KEY_PRESSED = 1,
        FNT_BLOCK_SIZE = 8,
        SCREEN_PARTION = 2 * FNT_BLOCK_SIZE,
        GRID_SIZE = 32,
        STEPS = 4,
        INVALID = -1,
        MAP_LEN = 255,
        MAP_HEI = 255,
    };

    enum {
        Key_Shift,
        Key_Ctrl,
        Key_Alt,
        Key_Enter,
        Key_Meta,
        Key_Count, // count
    };

    uint8_t m_keyStates[Key_Count];

};

#endif // CMAPSCROLL_H
