#include "SidebarPreviewPageEntry.h"

#include "control/Control.h"
#include "gui/sidebar/previews/base/SidebarPreviewBase.h"

SidebarPreviewPageEntry::SidebarPreviewPageEntry(SidebarPreviewPages* sidebar, const PageRef& page):
        SidebarPreviewBaseEntry(sidebar, page), sidebar(sidebar) {
    const auto clickCallback = G_CALLBACK(+[](GtkWidget* widget, GdkEvent* event, SidebarPreviewPageEntry* self) {
        // Open context menu on right mouse click
        if (event->type == GDK_BUTTON_PRESS) {
            auto mouseEvent = reinterpret_cast<GdkEventButton*>(event);
            if (mouseEvent->button == 3) {
                self->mouseButtonPressCallback();
                self->sidebar->openPreviewContextMenu();
                return true;
            }
        }
        return false;
    });
    g_signal_connect_after(this->widget, "button-press-event", clickCallback, this);
}

SidebarPreviewPageEntry::~SidebarPreviewPageEntry() = default;

auto SidebarPreviewPageEntry::getRenderType() -> PreviewRenderType { return RENDER_TYPE_PAGE_PREVIEW; }

void SidebarPreviewPageEntry::mouseButtonPressCallback() {
    const auto oldPageId = this->sidebar->getSelectedEntry();
    const auto newPageId = this->sidebar->getControl()->firePageSelected(this->page);
    // For better UX: don't scroll if clicked page is already selected
    if (oldPageId != newPageId) {
        this->sidebar->getControl()->getScrollHandler()->scrollToPage(this->page);
    }
}
