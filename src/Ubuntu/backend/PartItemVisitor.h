#ifndef UBUNTU_PARTITEMVISITOR_H
#define UBUNTU_PARTITEMVISITOR_H

#include "Ubuntu/backend/PartItemFactory.h"
#include "UiUtils/PartVisitor.h"

namespace Imap {
namespace Network {
class MsgPartNetAccessManager;
}
}

namespace UbuntuBackend {

class PartItemVisitor : public UiUtils::PartVisitor<QQuickItem *, QQuickItem *>
{
public:
    virtual ~PartItemVisitor();
    virtual QQuickItem * visitError(QString text, QQuickItem * parent);
    virtual QQuickItem * visitLoadablePart(QQuickItem * parent,
                                        Imap::Network::MsgPartNetAccessManager *manager,
                                        const QModelIndex &part,
                                        PartItemFactory *factory,
                                        int recursionDepth,
                                        const UiUtils::PartLoadingOptions loadingMode);
    virtual QQuickItem * visitAttachmentPart(QQuickItem * parent,
                                        Imap::Network::MsgPartNetAccessManager *manager,
                                        const QModelIndex &m_partIndex,
                                        QQuickItem * context, QQuickItem * contentView);
    virtual QQuickItem * visitMultipartAlternative(QQuickItem * parent,
                                        PartItemFactory *factory,
                                        const QModelIndex &partIndex,
                                        const int recursionDepth,
                                        const UiUtils::PartLoadingOptions options);
    virtual QQuickItem * visitMultipartSignedView(QQuickItem * parent, PartItemFactory *factory,
                                        const QModelIndex &partIndex, const int recursionDepth,
                                        const UiUtils::PartLoadingOptions loadingOptions);
    virtual QQuickItem * visitGenericMultipartView(QQuickItem * parent,
                                        PartItemFactory *factory,
                                        const QModelIndex &partIndex,
                                        int recursionDepth, const UiUtils::PartLoadingOptions options);
    virtual QQuickItem * visitMessage822View(QQuickItem * parent,
                                        PartItemFactory *factory, const QModelIndex &partIndex,
                                        int recursionDepth, const UiUtils::PartLoadingOptions options);
    virtual QQuickItem * visitSimplePartView(QQuickItem * parent, Imap::Network::MsgPartNetAccessManager *manager,
                                        const QModelIndex &partIndex,
                                        QQuickItem * messageView);
    virtual void applySetHidden(QQuickItem * view);
};

}

#endif // UBUNTU_PARTITEMVISITOR_H
