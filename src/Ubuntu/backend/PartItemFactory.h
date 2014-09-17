#ifndef UBUNTU_PARTITEMFACTORY_H
#define UBUNTU_PARTITEMFACTORY_H

#include <QQuickItem>
#include "UiUtils/PartWalker.h"

namespace UbuntuBackend {

typedef UiUtils::PartWalker<QQuickItem *, QQuickItem *> PartItemFactory;

}

namespace UiUtils {

extern template
QQuickItem *UbuntuBackend::PartItemFactory::walk(const QModelIndex &partIndex,
                    int recursionDepth, const PartLoadingOptions loadingMode);
extern template
QQuickItem *UbuntuBackend::PartItemFactory::context() const;
extern template
void UbuntuBackend::PartItemFactory::setNetworkWatcher(Imap::Mailbox::NetworkWatcher *netWatcher);

}

#endif // UBUNTU_PARTITEMFACTORY_H
