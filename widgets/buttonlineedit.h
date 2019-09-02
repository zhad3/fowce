#ifndef WIDGETSUTTONLINEEDIT_H
#define WIDGETSUTTONLINEEDIT_H

#include <QWidget>
#include <QVariant>
#include <QStyleOption>
#include <QTextLayout>

//class ButtonLineEditPrivate;
class ButtonLineEdit : public QWidget
{
    Q_OBJECT
    //Q_DECLARE_PRIVATE(ButtonLineEdit)
    //QScopedPointer<ButtonLineEditPrivate> const d_ptr;
public:
    explicit ButtonLineEdit(QWidget *parent = nullptr);
    QSize sizeHint() const override;

    QString text() const;
    void setText(const QString text);
    bool isReadOnly() const { return m_readOnly; }
    void setReadOnly(bool enable);
    bool isDown() const { return m_isDown; }
    QTextLayout *textLayout() const { return &m_textLayout; }
    int ascent() const { return m_ascent; }

    // QWidget interface
protected:
    void paintEvent(QPaintEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;


private:
    bool m_readOnly;
    bool m_isHover;
    bool m_isDown;
    int m_ascent;
    QRect m_contentRect;
    mutable QTextLayout m_textLayout;
    QString m_data;
    QAbstractItemModel *m_model;
    int m_topmargin;
    int m_rightmargin;
    int m_bottommargin;
    int m_leftmargin;

    void initStyleOptionButton(QStyleOptionButton *option) const;
    void initStyleOptionFrame(QStyleOptionFrame *option) const;
    int redoTextLayout() const;
    QSize sizeForWidth(int w) const;
};

#endif // WIDGETSUTTONLINEEDIT_H
