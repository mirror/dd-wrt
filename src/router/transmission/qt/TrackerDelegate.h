/*
 * This file Copyright (C) 2009-2015 Mnemosyne LLC
 *
 * It may be used under the GNU GPL versions 2 or 3
 * or any future license endorsed by Mnemosyne LLC.
 *
 * $Id: TrackerDelegate.h 14539 2015-06-12 22:12:12Z mikedld $
 */

#ifndef QTR_TRACKER_DELEGATE_H
#define QTR_TRACKER_DELEGATE_H

#include <QItemDelegate>

class QStyle;

class Session;
struct TrackerInfo;

class TrackerDelegate: public QItemDelegate
{
    Q_OBJECT

  public:
    TrackerDelegate (QObject * parent = nullptr): QItemDelegate (parent), myShowMore (false) {}
    virtual ~TrackerDelegate () {}

    void setShowMore (bool b);

    // QAbstractItemDelegate
    virtual QSize sizeHint (const QStyleOptionViewItem& option, const QModelIndex& index) const;
    virtual void paint (QPainter * painter, const QStyleOptionViewItem& option, const QModelIndex& index) const;

  protected:
    QString getText (const TrackerInfo&) const;
    QSize margin (const QStyle& style) const;

    QSize sizeHint (const QStyleOptionViewItem&, const TrackerInfo&) const;
    void drawTracker (QPainter *, const QStyleOptionViewItem&, const TrackerInfo&) const;

  private:
    bool myShowMore;
};

#endif // QTR_TRACKER_DELEGATE_H
