#include "PartItemFactory.h"
#include "UiUtils/PartWalker_impl.h"

namespace UiUtils {

template UbuntuBackend::PartItemFactory::PartWalker(Imap::Network::MsgPartNetAccessManager *manager,
                    QQuickItem *context,
std::unique_ptr<UiUtils::PartVisitor<QQuickItem *, QQuickItem *>> visitor);
template QQuickItem *UbuntuBackend::PartItemFactory::walk(const QModelIndex &partIndex,
                    int recursionDepth, const PartLoadingOptions loadingMode);
template QQuickItem *UbuntuBackend::PartItemFactory::context() const;
template void UbuntuBackend::PartItemFactory::setNetworkWatcher(Imap::Mailbox::NetworkWatcher *netWatcher);

}
