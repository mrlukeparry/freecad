#ifndef LINEEDIT_H
#define LINEEDIT_H

#include <QLineEdit>

class QToolButton;

namespace PartDesignGui {

class SelectionLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    enum EditMode{
        STATUS_SELECT,
        STATUS_VALID,
        STATUS_INVALID,
        STATUS_EMPTY};

    SelectionLineEdit(QWidget *parent = 0);
    ~SelectionLineEdit();
    void updateStatus(const EditMode &mode);

protected:
    void resizeEvent(QResizeEvent *);
    bool event(QEvent *event);

private Q_SLOTS:
    void updateCloseButton(const QString &text);

private:
    QToolButton *clearButton;
};

}

#endif // LIENEDIT_H
