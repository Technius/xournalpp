/*
 * Xournal++
 *
 * Caches PDF backgrounds for faster repaint
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <list>
#include <string>
#include <vector>

#include <cairo/cairo.h>
#include <gdk/gdk.h>

#include "pdf/base/XojPdfPage.h"

#include "XournalType.h"
using std::list;

class PdfCacheEntry;

class PdfCache {
public:
    PdfCache(int size, GdkWindow* window);
    virtual ~PdfCache();

private:
    PdfCache(const PdfCache& cache) = delete;
    void operator=(const PdfCache& cache) = delete;
    PdfCache(const PdfCache&& cache) = delete;
    void operator=(const PdfCache&& cache) = delete;

public:
    void render(cairo_t* cr, const XojPdfPageSPtr& popplerPage, double zoom);
    void clearCache();

private:
    void setZoom(double zoom);
    cairo_surface_t* lookup(const XojPdfPageSPtr& popplerPage);
    void cache(XojPdfPageSPtr popplerPage, cairo_surface_t* img);

private:
    GMutex renderMutex{};

    list<PdfCacheEntry*> data;
    list<PdfCacheEntry*>::size_type size = 0;
    GdkWindow* window = nullptr;  // only needed for faster rendering path

    double zoom = -1;
};
