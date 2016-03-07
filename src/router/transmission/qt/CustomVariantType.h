/*
 * This file Copyright (C) 2012-2015 Mnemosyne LLC
 *
 * It may be used under the GNU GPL versions 2 or 3
 * or any future license endorsed by Mnemosyne LLC.
 *
 * $Id: CustomVariantType.h 14539 2015-06-12 22:12:12Z mikedld $
 */

#ifndef QTR_TYPES_H
#define QTR_TYPES_H

#include <QVariant>

class CustomVariantType
{
  public:
    enum
    {
      TrackerStatsList = QVariant::UserType,
      PeerList = QVariant::UserType,
      FileList,
      FilterModeType,
      SortModeType
    };
};

#endif // QTR_TYPES_H
