/*
 * Xournal++
 *
 * PDF Page GLib Implementation
 *
 * @author Xournal++ Team
 * https://github.com/xournalpp/xournalpp
 *
 * @license GNU GPLv2 or later
 */

#pragma once

#include <poppler.h>

#include "pdf/base/XojPdfPage.h"


class PopplerGlibPage: public XojPdfPage {
public:
    PopplerGlibPage(PopplerPage* page, PopplerDocument* doc);
    PopplerGlibPage(const PopplerGlibPage& other);
    virtual ~PopplerGlibPage();
    PopplerGlibPage& operator=(const PopplerGlibPage& other);

public:
    virtual double getWidth() const override;
    virtual double getHeight() const override;

    virtual void render(cairo_t* cr, bool forPrinting = false) override;  // NOLINT(google-default-arguments)

    virtual vector<XojPdfRectangle> findText(string& text) override;

    auto getLinks() -> std::vector<Link> override;

    virtual int getPageId() override;

private:
    PopplerPage* page;
    PopplerDocument* document;
};
