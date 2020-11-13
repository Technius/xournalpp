#include "PdfCache.h"

#include <cstdio>
#include <utility>

class PdfCacheEntry {
public:
    PdfCacheEntry(XojPdfPageSPtr popplerPage, cairo_surface_t* img) {
        this->popplerPage = std::move(popplerPage);
        this->rendered = img;
    }

    ~PdfCacheEntry() {
        this->popplerPage = nullptr;
        cairo_surface_destroy(this->rendered);
        this->rendered = nullptr;
    }

    PdfCacheEntry(const PdfCacheEntry& cache) = delete;
    void operator=(const PdfCacheEntry& cache) = delete;
    PdfCacheEntry(const PdfCacheEntry&& cache) = delete;
    void operator=(const PdfCacheEntry&& cache) = delete;

    XojPdfPageSPtr popplerPage;
    cairo_surface_t* rendered;
};

PdfCache::PdfCache(int size, GdkWindow* window): window(window) {
    this->size = size;

    g_mutex_init(&this->renderMutex);
}

PdfCache::~PdfCache() {
    clearCache();
    this->size = 0;
}

void PdfCache::setZoom(double zoom) {
    if (this->zoom == zoom) {
        return;
    }
    this->zoom = zoom;

    clearCache();
}

void PdfCache::clearCache() {
    for (PdfCacheEntry* e: this->data) {
        delete e;
    }
    this->data.clear();
}

auto PdfCache::lookup(const XojPdfPageSPtr& popplerPage) -> cairo_surface_t* {
    for (PdfCacheEntry* e: this->data) {
        if (e->popplerPage->getPageId() == popplerPage->getPageId()) {
            return e->rendered;
        }
    }

    return nullptr;
}

void PdfCache::cache(XojPdfPageSPtr popplerPage, cairo_surface_t* img) {
    auto* ne = new PdfCacheEntry(std::move(popplerPage), img);
    this->data.push_front(ne);

    while (this->data.size() > this->size) {
        delete this->data.back();
        this->data.pop_back();
    }
}

void PdfCache::render(cairo_t* cr, const XojPdfPageSPtr& popplerPage, double zoom) {
    g_mutex_lock(&this->renderMutex);

    this->setZoom(zoom);

    cairo_surface_t* img = lookup(popplerPage);
    auto surfaceWidth = static_cast<int>(popplerPage->getWidth() * this->zoom);
    auto surfaceHeight = static_cast<int>(popplerPage->getHeight() * this->zoom);
    if (img == nullptr) {
        if (this->window) {
            // img = gdk_window_create_similar_image_surface(this->window, CAIRO_FORMAT_ARGB32, surfaceWidth,
            // surfaceHeight, 0);
            img = gdk_window_create_similar_surface(this->window, CAIRO_CONTENT_COLOR_ALPHA, surfaceWidth,
                                                    surfaceHeight);
        } else {
            img = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, surfaceWidth, surfaceHeight);
        }
        cairo_t* cr2 = cairo_create(img);

        cairo_scale(cr2, this->zoom, this->zoom);
        popplerPage->render(cr2, false);
        cairo_destroy(cr2);
        cache(popplerPage, img);
    }

    cairo_matrix_t mOriginal;
    cairo_matrix_t mScaled;
    cairo_get_matrix(cr, &mOriginal);
    cairo_get_matrix(cr, &mScaled);
    mScaled.xx = 1;
    mScaled.yy = 1;
    mScaled.xy = 0;
    mScaled.yx = 0;
    cairo_set_matrix(cr, &mScaled);
    cairo_set_source_surface(cr, img, 0, 0);
    cairo_paint(cr);
    cairo_set_matrix(cr, &mOriginal);

    g_mutex_unlock(&this->renderMutex);
}
