/* Copyright (C) 2006 - 2014 Jan Kundrát <jkt@flaska.net>

   This file is part of the Trojita Qt IMAP e-mail client,
   http://trojita.flaska.net/

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License or (at your option) version 3 or any later version
   accepted by the membership of KDE e.V. (or its successor approved
   by the membership of KDE e.V.), which shall act as a proxy
   defined in Section 14 of version 3 of the license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef UIUTILS_PARTVISITOR_H
#define UIUTILS_PARTVISITOR_H

#include "PartWalker.h"
#include "PartLoadingOptions.h"

namespace UiUtils {

template <typename T> class PartWalker;

template <typename T>
class PartVisitor
{
    typedef T result_type;
public:
    virtual ~PartVisitor() { }
    virtual result_type *visitError(QString text, T *parent) = 0;
    virtual result_type *visitLoadablePart(T *parent, Imap::Network::MsgPartNetAccessManager *manager, const QModelIndex &part,
                                            PartWalker<T> *factory, int recursionDepth,
                                            const UiUtils::PartLoadingOptions loadingMode) = 0;
    virtual result_type *visitAttachmentPart(T *parent, Imap::Network::MsgPartNetAccessManager *manager, const QModelIndex &m_partIndex,
                                      T *messageView, T *contentView) = 0;
    virtual result_type *visitMultipartAlternative(T *parent, PartWalker<T> *factory, const QModelIndex &partIndex,
                                                  const int recursionDepth, const UiUtils::PartLoadingOptions options) = 0;
    virtual result_type *visitMultipartSignedView(T *parent, PartWalker<T> *factory, const QModelIndex &partIndex, const int recursionDepth,
                                              const UiUtils::PartLoadingOptions loadingOptions) = 0;
    virtual result_type *visitGenericMultipartView(T *parent,
                                              PartWalker<T> *factory, const QModelIndex &partIndex,
                                              int recursionDepth, const UiUtils::PartLoadingOptions options) = 0;
    virtual result_type *visitMessage822View(T *parent,
                                 PartWalker<T> *factory, const QModelIndex &partIndex,
                                 int recursionDepth, const UiUtils::PartLoadingOptions options) = 0;
    virtual result_type *visitSimplePartView(T *parent, Imap::Network::MsgPartNetAccessManager *manager, const QModelIndex &partIndex,
                                 T *messageView) = 0;
    virtual void hideView(T *view) = 0;
};

}

#endif // UIUTILS_PARTVISITOR_H
