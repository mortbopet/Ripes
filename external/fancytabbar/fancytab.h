#ifndef FANCYTAB_H
#define FANCYTAB_H

#include <QIcon>
#include <QString>

/*!
 * \brief The FancyTab class is more like struct than class. It has constructor
 *   to set tab icon and tab text and the the icon and tab are accesseble
 *   directly.
 */
class FancyTab {
public:
  FancyTab(const QIcon &icon, QString text);

  QString m_text;
  QIcon m_icon;
};

#endif // FANCYTAB_H
