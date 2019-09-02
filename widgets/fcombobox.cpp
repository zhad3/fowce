#include "fcombobox.h"

bool FComboBox::event(QEvent *event)
{
    m_previousIndex = currentIndex();
    return QComboBox::event(event);
}
