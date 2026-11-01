#include "PluginEditor.h"
#include <BinaryData.h>

namespace steso::ui
{
//==============================================================================
juce::Font panelFont (float height, bool bold)
{
    static juce::Typeface::Ptr boldFace = juce::Typeface::createSystemTypefaceFor (
        StesoAssets::PT_SansNarrowWebBold_ttf, (size_t) StesoAssets::PT_SansNarrowWebBold_ttfSize);
    static juce::Typeface::Ptr regularFace = juce::Typeface::createSystemTypefaceFor (
        StesoAssets::PT_SansNarrowWebRegular_ttf, (size_t) StesoAssets::PT_SansNarrowWebRegular_ttfSize);
    return juce::Font (juce::FontOptions (bold ? boldFace : regularFace).withHeight (height));
}

void drawEngravedBox (juce::Graphics& g, juce::Rectangle<float> r, float alpha)
{
    g.setColour (colours::engraveDark.withAlpha (0.9f * alpha));
    g.drawLine (r.getX(), r.getY(), r.getRight(), r.getY(), 1.2f);
    g.drawLine (r.getX(), r.getY(), r.getX(), r.getBottom(), 1.2f);
    g.setColour (colours::engraveLight.withAlpha (0.55f * alpha));
    g.drawLine (r.getX() + 1.0f, r.getBottom(), r.getRight(), r.getBottom(), 1.0f);
    g.drawLine (r.getRight(), r.getY() + 1.0f, r.getRight(), r.getBottom(), 1.0f);
}

void drawScrew (juce::Graphics& g, juce::Point<float> c, float radius, float slotAngle)
{
    g.setColour (colours::engraveDark.withAlpha (0.8f));
    g.fillEllipse (c.x - radius - 1.0f, c.y - radius - 1.0f, radius * 2.0f + 2.0f, radius * 2.0f + 2.0f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff8a8a82), c.x - radius, c.y - radius,
                                             juce::Colour (0xff3e3e3a), c.x + radius, c.y + radius, false));
    g.fillEllipse (c.x - radius, c.y - radius, radius * 2.0f, radius * 2.0f);
    juce::Path slot;
    slot.addRectangle (-radius * 0.85f, -0.9f, radius * 1.7f, 1.8f);
    g.setColour (juce::Colour (0xff1c1c1a));
    g.fillPath (slot, juce::AffineTransform::rotation (slotAngle).translated (c.x, c.y));
}

void drawLamp (juce::Graphics& g, juce::Rectangle<float> r, juce::Colour lit, juce::Colour off, bool on, float alpha)
{
    const auto c = r.getCentre();
    const float rad = std::min (r.getWidth(), r.getHeight()) * 0.5f;
    // Infattning
    g.setColour (juce::Colour (0xff0e0e0d).withAlpha (alpha));
    g.fillEllipse (c.x - rad, c.y - rad, rad * 2.0f, rad * 2.0f);
    const float lr = rad * 0.72f;
    if (on)
    {
        g.setGradientFill (juce::ColourGradient (lit.brighter (0.6f).withAlpha (alpha), c.x - lr * 0.3f, c.y - lr * 0.3f,
                                                 lit.darker (0.5f).withAlpha (alpha), c.x + lr, c.y + lr, true));
        g.fillEllipse (c.x - lr, c.y - lr, lr * 2.0f, lr * 2.0f);
        g.setColour (lit.withAlpha (0.18f * alpha));
        g.fillEllipse (c.x - rad * 1.25f, c.y - rad * 1.25f, rad * 2.5f, rad * 2.5f);
    }
    else
    {
        g.setGradientFill (juce::ColourGradient (off.brighter (0.25f).withAlpha (alpha), c.x - lr * 0.3f, c.y - lr * 0.3f,
                                                 off.darker (0.4f).withAlpha (alpha), c.x + lr, c.y + lr, true));
        g.fillEllipse (c.x - lr, c.y - lr, lr * 2.0f, lr * 2.0f);
    }
    // Glasreflex
    g.setColour (juce::Colours::white.withAlpha ((on ? 0.35f : 0.12f) * alpha));
    g.fillEllipse (c.x - lr * 0.5f, c.y - lr * 0.6f, lr * 0.5f, lr * 0.35f);
}

void drawSectionFrame (juce::Graphics& g, juce::Rectangle<float> r, const juce::String& title, const juce::String& note)
{
    // Inga boxar: bara en tunn tryckt linje till höger och under sektionen (som på en gammal frontpanel).
    {
        const auto outer = r.expanded (3.0f);
        g.setColour (colours::hairline.withAlpha (0.22f));
        g.drawLine (outer.getRight() - 0.5f, outer.getY() + 6.0f, outer.getRight() - 0.5f, outer.getBottom() - 6.0f, 1.0f);
        g.drawLine (outer.getX() + 6.0f, outer.getBottom() - 0.5f, outer.getRight() - 6.0f, outer.getBottom() - 0.5f, 1.0f);
    }

    const auto cyr = title.upToFirstOccurrenceOf ("|", false, false);
    const auto lat = title.fromFirstOccurrenceOf ("|", false, false);

    auto header = r.removeFromTop (24.0f).reduced (10.0f, 0.0f).withTrimmedTop (3.0f);

    // Krymp rubriken tills svenska + (engelska) får plats i smala sektioner.
    const auto latUpper = lat.isNotEmpty() ? "(" + lat.toUpperCase() + ")" : juce::String();
    const auto noteFont = panelFont (12.0f, false).withExtraKerningFactor (0.1f);
    const float noteW = note.isNotEmpty() ? juce::GlyphArrangement::getStringWidth (noteFont, note) + 10.0f : 0.0f;
    float size = 14.0f;
    auto titleFont = [] (float sz) { return panelFont (sz, false).withExtraKerningFactor (0.16f); };
    auto subFont = [] (float sz) { return panelFont (sz * 0.72f, false).withExtraKerningFactor (0.14f); };
    auto fits = [&] (float sz)
    {
        const float w1 = juce::GlyphArrangement::getStringWidth (titleFont (sz), cyr);
        const float w2 = latUpper.isEmpty() ? 0.0f : juce::GlyphArrangement::getStringWidth (subFont (sz), latUpper) + 8.0f;
        return w1 + w2 + 12.0f + noteW <= header.getWidth();
    };
    while (size > 9.5f && ! fits (size))
        size -= 0.5f;
    const auto f1 = titleFont (size);
    g.setFont (f1);
    g.setColour (colours::text);
    g.drawText (cyr, header, juce::Justification::centredLeft);
    float x = header.getX() + juce::GlyphArrangement::getStringWidth (f1, cyr) + 8.0f;

    if (lat.isNotEmpty() && fits (size))
    {
        const auto f2 = subFont (size);
        g.setFont (f2);
        g.setColour (colours::dimText);
        g.drawText (latUpper, juce::Rectangle<float> (x, header.getY() + 1.0f, 220.0f, header.getHeight()),
                    juce::Justification::centredLeft);
    }

    // Notering längst till höger (t.ex. röster).
    if (note.isNotEmpty())
    {
        const auto f3 = noteFont;
        const float nw = juce::GlyphArrangement::getStringWidth (f3, note) + 2.0f;
        g.setFont (f3);
        g.setColour (colours::lampAmber.withAlpha (0.85f));
        g.drawText (note, juce::Rectangle<float> (header.getRight() - nw, header.getY() + 1.0f, nw, header.getHeight()),
                    juce::Justification::centredRight);
    }
}

void drawRackEar (juce::Graphics& g, juce::Rectangle<float> r)
{
    // Böjd plåt: lite ljusare än panelen, mörk fog mot panelen.
    g.setGradientFill (juce::ColourGradient (colours::metal.brighter (0.12f), r.getX(), r.getY(),
                                             colours::metalDark, r.getRight(), r.getBottom(), false));
    g.fillRect (r);
    g.setColour (colours::engraveLight.withAlpha (0.5f));
    g.drawLine (r.getX() + 1.0f, r.getY(), r.getX() + 1.0f, r.getBottom(), 1.0f);
    g.setColour (colours::engraveDark);
    g.drawLine (r.getRight() - 0.5f, r.getY(), r.getRight() - 0.5f, r.getBottom(), 1.5f);
    g.drawLine (r.getX(), r.getBottom() - 0.5f, r.getRight(), r.getBottom() - 0.5f, 1.0f);

    // Ovala rackhål med skruv: upptill och nedtill (och i mitten på höga enheter).
    std::vector<float> ys { r.getY() + 22.0f, r.getBottom() - 22.0f };
    if (r.getHeight() > 300.0f)
        ys.push_back (r.getCentreY());
    for (float y : ys)
    {
        auto slot = juce::Rectangle<float> (r.getCentreX() - 9.0f, y - 6.0f, 18.0f, 12.0f);
        g.setColour (juce::Colour (0xff0b0b0a));
        g.fillRoundedRectangle (slot, 6.0f);
        g.setColour (colours::engraveLight.withAlpha (0.4f));
        g.drawRoundedRectangle (slot.translated (0.0f, 0.8f), 6.0f, 0.8f);
        drawScrew (g, { r.getCentreX() + 2.0f, y }, 5.0f, 0.4f + y * 0.013f);
    }
}

namespace
{
    // Mjuka fläckar i klungor (rost, ärg, sot) – många små ovaler med låg täckning.
    void blotch (juce::Graphics& g, juce::Random& rnd, float cx, float cy, float size, juce::Colour col, int count, float alpha)
    {
        for (int k = 0; k < count; ++k)
        {
            const float a = rnd.nextFloat() * juce::MathConstants<float>::twoPi;
            const float d = std::pow (rnd.nextFloat(), 0.7f) * size;
            const float x = cx + std::cos (a) * d, y = cy + std::sin (a) * d * 0.8f;
            const float r = 1.0f + std::pow (rnd.nextFloat(), 3.0f) * size * 0.35f;
            g.setGradientFill (juce::ColourGradient (col.withAlpha (alpha * (0.4f + rnd.nextFloat())), x, y,
                                                     col.withAlpha (0.0f), x + r, y, true));
            g.fillEllipse (x - r, y - r, r * 2.0f, r * 2.0f);
        }
    }
}

juce::Image makePanelTexture (int w, int h)
{
    juce::Image img (juce::Image::RGB, w, h, false);
    juce::Random rnd (19611961);   // fast frö: samma slitage varje gång

    {
        // Kall, grovkornig plåt: korn + svaga horisontella slipspår.
        juce::Image::BitmapData data (img, juce::Image::BitmapData::writeOnly);
        const auto base = colours::metal;
        for (int y = 0; y < h; ++y)
        {
            const float streak = (float) (rnd.nextInt (7) - 3) * 0.5f;
            for (int x = 0; x < w; ++x)
            {
                const float n = (rnd.nextFloat() - 0.5f) * 11.0f + streak;
                const auto px = juce::Colour::fromRGB ((juce::uint8) juce::jlimit (0, 255, (int) (base.getRed() + n * 0.95f)),
                                                       (juce::uint8) juce::jlimit (0, 255, (int) (base.getGreen() + n)),
                                                       (juce::uint8) juce::jlimit (0, 255, (int) (base.getBlue() + n * 1.08f)));
                data.setPixelColour (x, y, px);
            }
        }
    }

    juce::Graphics g (img);

    // Sot och nötta, ljusare partier i stor skala.
    for (int i = 0; i < 70; ++i)
        blotch (g, rnd, rnd.nextFloat() * (float) w, rnd.nextFloat() * (float) h, 60.0f + rnd.nextFloat() * 160.0f,
                juce::Colour (0xff000000), 10, 0.10f);
    for (int i = 0; i < 25; ++i)
        blotch (g, rnd, rnd.nextFloat() * (float) w, rnd.nextFloat() * (float) h, 40.0f + rnd.nextFloat() * 120.0f,
                juce::Colour (0xff4a5258), 8, 0.05f);

    // Rost: brunröda klungor, tätast nära kanter och i hörn.
    for (int i = 0; i < 38; ++i)
    {
        const bool edge = rnd.nextFloat() < 0.6f;
        const float x = edge ? (rnd.nextBool() ? rnd.nextFloat() * 80.0f : (float) w - rnd.nextFloat() * 80.0f) : rnd.nextFloat() * (float) w;
        const float y = rnd.nextFloat() * (float) h;
        blotch (g, rnd, x, y, 10.0f + rnd.nextFloat() * 45.0f, juce::Colour (0xff5a2c14), 40, 0.16f);
        blotch (g, rnd, x, y, 6.0f + rnd.nextFloat() * 20.0f, juce::Colour (0xff8a4a1e), 18, 0.10f);
    }
    // Ärg: kallt blågröna stänk.
    for (int i = 0; i < 22; ++i)
        blotch (g, rnd, rnd.nextFloat() * (float) w, rnd.nextFloat() * (float) h, 8.0f + rnd.nextFloat() * 30.0f,
                juce::Colour (0xff3f6a66), 30, 0.10f);

    // Sprickor i lacken.
    for (int i = 0; i < 26; ++i)
    {
        juce::Path crack;
        float x = rnd.nextFloat() * (float) w, y = rnd.nextFloat() * (float) h;
        float ang = rnd.nextFloat() * juce::MathConstants<float>::twoPi;
        crack.startNewSubPath (x, y);
        const int segs = 5 + rnd.nextInt (26);
        for (int k = 0; k < segs; ++k)
        {
            ang += (rnd.nextFloat() - 0.5f) * 1.2f;
            const float len = 4.0f + rnd.nextFloat() * 15.0f;
            x += std::cos (ang) * len;
            y += std::sin (ang) * len;
            crack.lineTo (x, y);
            if (rnd.nextFloat() < 0.2f)
            {
                float bx = x, by = y, ba = ang + (rnd.nextBool() ? 1.0f : -1.0f);
                crack.startNewSubPath (bx, by);
                for (int m = 0; m < 2 + rnd.nextInt (5); ++m)
                {
                    ba += (rnd.nextFloat() - 0.5f) * 1.0f;
                    bx += std::cos (ba) * (3.0f + rnd.nextFloat() * 8.0f);
                    by += std::sin (ba) * (3.0f + rnd.nextFloat() * 8.0f);
                    crack.lineTo (bx, by);
                }
                crack.startNewSubPath (x, y);
            }
        }
        g.setColour (juce::Colour (0xff9aa3a8).withAlpha (0.13f));
        g.strokePath (crack, juce::PathStrokeType (0.7f), juce::AffineTransform::translation (0.6f, 0.8f));
        g.setColour (juce::Colours::black.withAlpha (0.8f));
        g.strokePath (crack, juce::PathStrokeType (1.0f));
    }

    // Repor.
    for (int i = 0; i < 420; ++i)
    {
        const float x1 = rnd.nextFloat() * (float) w, y1 = rnd.nextFloat() * (float) h;
        const float len = 3.0f + std::pow (rnd.nextFloat(), 2.0f) * 80.0f;
        const float a = rnd.nextFloat() * juce::MathConstants<float>::twoPi;
        g.setColour (juce::Colour (0xffaab3b8).withAlpha (0.025f + rnd.nextFloat() * 0.07f));
        g.drawLine (x1, y1, x1 + std::cos (a) * len, y1 + std::sin (a) * len, 0.5f + rnd.nextFloat() * 0.5f);
    }
    return img;
}

juce::Image makeGrimeOverlay (int w, int h)
{
    juce::Image img (juce::Image::ARGB, w, h, true);
    juce::Random rnd (4242);
    {
        // Filmkorn: glesa mörka och ljusa prickar över allt.
        juce::Image::BitmapData data (img, juce::Image::BitmapData::readWrite);
        for (int y = 0; y < h; ++y)
            for (int x = 0; x < w; ++x)
            {
                const float r = rnd.nextFloat();
                if (r < 0.10f)
                    data.setPixelColour (x, y, juce::Colour (0, 0, 0).withAlpha ((juce::uint8) (40 + rnd.nextInt (70))));
                else if (r > 0.985f)
                    data.setPixelColour (x, y, juce::Colour (190, 200, 205).withAlpha ((juce::uint8) (10 + rnd.nextInt (25))));
            }
    }
    juce::Graphics g (img);
    // Damm och smutsfläckar som äter på text och rattar.
    for (int i = 0; i < 1600; ++i)
    {
        const float x = rnd.nextFloat() * (float) w, y = rnd.nextFloat() * (float) h;
        const float r = 0.4f + std::pow (rnd.nextFloat(), 4.0f) * 2.6f;
        g.setColour ((rnd.nextFloat() < 0.8f ? juce::Colour (0xff000000) : juce::Colour (0xff8a9298)).withAlpha (0.2f + rnd.nextFloat() * 0.4f));
        g.fillEllipse (x - r, y - r, r * 2.0f, r * 2.0f);
    }
    for (int i = 0; i < 60; ++i)
        blotch (g, rnd, rnd.nextFloat() * (float) w, rnd.nextFloat() * (float) h, 20.0f + rnd.nextFloat() * 70.0f,
                juce::Colour (0xff000000), 12, 0.09f);
    return img;
}

void drawSpotLight (juce::Graphics& g, juce::Rectangle<float> area, juce::Point<float> spot)
{
    // Ljuskägla: genomskinlig i mitten, svart ut mot kanterna (elliptisk – bredare än hög).
    const float rx = area.getWidth() * 0.62f;
    juce::ColourGradient cone (juce::Colours::transparentBlack, spot.x, spot.y,
                               juce::Colours::black.withAlpha (0.8f), spot.x + rx, spot.y, true);
    cone.addColour (0.3, juce::Colours::black.withAlpha (0.04f));
    cone.addColour (0.6, juce::Colours::black.withAlpha (0.32f));
    cone.addColour (0.85, juce::Colours::black.withAlpha (0.6f));
    g.saveState();
    g.addTransform (juce::AffineTransform::scale (1.0f, 0.78f, spot.x, spot.y));
    g.setGradientFill (cone);
    g.fillRect (area.withY (area.getY() - area.getHeight()).withHeight (area.getHeight() * 4.0f));
    g.restoreState();
    // Själva ljuspölen: svagt kallt sken där ljuset träffar.
    {
        const float r = area.getWidth() * 0.32f;
        g.saveState();
        g.addTransform (juce::AffineTransform::scale (1.0f, 0.55f, spot.x, spot.y));
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xffc9dbe8).withAlpha (0.07f), spot.x, spot.y,
                                                 juce::Colour (0xffc9dbe8).withAlpha (0.0f), spot.x + r, spot.y, true));
        g.fillEllipse (spot.x - r, spot.y - r, r * 2.0f, r * 2.0f);
        g.restoreState();
    }
    // Kallt släpljus: en aning blå ton över alltihop.
    g.setColour (juce::Colour (0xff0a1a2a).withAlpha (0.18f));
    g.fillRect (area);
}

//==============================================================================
LookAndFeel::LookAndFeel()
{
    setColour (juce::ComboBox::backgroundColourId, colours::bakelite);
    setColour (juce::ComboBox::outlineColourId, colours::engraveDark);
    setColour (juce::ComboBox::textColourId, colours::text);
    setColour (juce::ComboBox::arrowColourId, colours::dimText);
    setColour (juce::PopupMenu::backgroundColourId, colours::metalDark);
    setColour (juce::PopupMenu::textColourId, colours::text);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, colours::lampAmber.withAlpha (0.85f));
    setColour (juce::PopupMenu::highlightedTextColourId, colours::bakelite);
    setColour (juce::PopupMenu::headerTextColourId, colours::lampAmber);
    setColour (juce::Label::textColourId, colours::text);
    setColour (juce::TextButton::buttonColourId, colours::metal);
    setColour (juce::TextButton::textColourOffId, colours::text);
    setColour (juce::TextButton::textColourOnId, colours::text);
    setColour (juce::Slider::textBoxBackgroundColourId, colours::bakelite);
    setColour (juce::Slider::textBoxTextColourId, colours::text);
    setColour (juce::Slider::textBoxOutlineColourId, colours::engraveDark);
    setColour (juce::BubbleComponent::backgroundColourId, colours::bakelite.withAlpha (0.95f));
    setColour (juce::BubbleComponent::outlineColourId, colours::dimText);
    setColour (juce::TooltipWindow::backgroundColourId, colours::bakelite);
    setColour (juce::TooltipWindow::textColourId, colours::text);
    setColour (juce::TooltipWindow::outlineColourId, colours::dimText);
}

juce::Typeface::Ptr LookAndFeel::getTypefaceForFont (const juce::Font& f)
{
    return panelFont (f.getHeight(), f.isBold()).getTypefacePtr();
}

void LookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height, float pos,
                                    float startAngle, float endAngle, juce::Slider& slider)
{
    const auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (2.0f);
    const float radius = std::min (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto c = bounds.getCentre();
    const float angle = startAngle + pos * (endAngle - startAngle);

    // Tunna tryckta skalstreck runt ratten.
    const bool bipolar = slider.getMinimum() < 0.0 && juce::approximatelyEqual (slider.getMinimum(), -slider.getMaximum());
    for (int i = 0; i <= 10; ++i)
    {
        const float a = startAngle + (endAngle - startAngle) * (float) i / 10.0f;
        const bool major = i == 0 || i == 10 || (bipolar && i == 5);
        const float r1 = radius * (major ? 0.82f : 0.87f), r2 = radius * 0.97f;
        g.setColour (colours::text.withAlpha (major ? 0.55f : 0.32f));
        g.drawLine (c.x + std::sin (a) * r1, c.y - std::cos (a) * r1, c.x + std::sin (a) * r2, c.y - std::cos (a) * r2, 0.9f);
    }

    // Skugga under ratten.
    const float skirt = radius * 0.76f;
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.fillEllipse (c.x - skirt + 1.0f, c.y - skirt + 3.0f, skirt * 2.0f, skirt * 2.0f);

    // Svart räfflad kjol.
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff2a2a28), c.x, c.y - skirt,
                                             juce::Colour (0xff050505), c.x, c.y + skirt, false));
    g.fillEllipse (c.x - skirt, c.y - skirt, skirt * 2.0f, skirt * 2.0f);
    for (int i = 0; i < 30; ++i)
    {
        const float a = angle + juce::MathConstants<float>::twoPi * (float) i / 30.0f;
        g.setColour (juce::Colours::white.withAlpha (0.035f + 0.04f * std::max (0.0f, -std::cos (a))));
        g.drawLine (c.x + std::sin (a) * skirt * 0.84f, c.y - std::cos (a) * skirt * 0.84f,
                    c.x + std::sin (a) * skirt * 0.99f, c.y - std::cos (a) * skirt * 0.99f, 0.9f);
    }

    // Varje ratt får eget slitage (fast per parameter, så att det ser likadant ut varje gång).
    juce::Random wear ((juce::int64) slider.getName().hashCode() * 7919 + 17);
    const float corrosion = wear.nextFloat() < 0.45f ? 0.3f + wear.nextFloat() * 0.7f : 0.0f;
    if (corrosion > 0.0f)
    {
        // Ärgskorpa runt kjolens fot: blågröna och rostbruna korn.
        const float clumpA = wear.nextFloat() * juce::MathConstants<float>::twoPi;   // skorpan samlas på en sida
        for (int k = 0; k < (int) (90 * corrosion); ++k)
        {
            const float a = clumpA + (wear.nextFloat() - 0.5f) * (wear.nextFloat() < 0.75f ? 2.2f : 6.3f);
            const float d = skirt * (0.92f + std::pow (wear.nextFloat(), 1.8f) * 0.32f);
            const float rr = 0.4f + wear.nextFloat() * 1.3f;
            const auto col = wear.nextFloat() < 0.7f ? juce::Colour (0xff4f8a84) : juce::Colour (0xff7a4a26);
            g.setColour (col.withMultipliedBrightness (0.6f + wear.nextFloat() * 0.5f).withAlpha ((0.2f + wear.nextFloat() * 0.45f) * corrosion));
            g.fillEllipse (c.x + std::sin (a) * d - rr, c.y - std::cos (a) * d - rr, rr * 2.0f, rr * 2.0f);
        }
    }

    // Kall, sliten ståltopp: koncentriska slipspår + matt reflex uppifrån vänster.
    const float cap = skirt * 0.60f;
    const float tone = 0.75f + wear.nextFloat() * 0.35f;
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff6c7276).withMultipliedBrightness (tone), c.x - cap * 0.6f, c.y - cap * 0.8f,
                                             juce::Colour (0xff121315), c.x + cap * 0.6f, c.y + cap * 0.9f, false));
    g.fillEllipse (c.x - cap, c.y - cap, cap * 2.0f, cap * 2.0f);
    for (float rr = cap * 0.12f; rr < cap; rr += 1.2f)
    {
        g.setColour ((((int) (rr * 5.0f)) % 2 == 0 ? juce::Colours::white : juce::Colours::black).withAlpha (0.04f));
        g.drawEllipse (c.x - rr, c.y - rr, rr * 2.0f, rr * 2.0f, 0.6f);
    }
    {
        juce::Path wedge;
        wedge.addPieSegment (c.x - cap, c.y - cap, cap * 2.0f, cap * 2.0f, -2.5f, -2.0f, 0.0f);
        g.setColour (juce::Colour (0xffd8e4ec).withAlpha (0.16f));
        g.fillPath (wedge);
    }
    // Repor och fläckar på toppen.
    for (int k = 0; k < 3 + wear.nextInt (6); ++k)
    {
        const float a = wear.nextFloat() * juce::MathConstants<float>::twoPi, d = wear.nextFloat() * cap * 0.8f;
        const float px = c.x + std::cos (a) * d, py = c.y + std::sin (a) * d;
        const float la = wear.nextFloat() * juce::MathConstants<float>::twoPi, len = 1.5f + wear.nextFloat() * cap * 0.6f;
        g.setColour ((wear.nextBool() ? juce::Colours::black : juce::Colour (0xffc8cfd2)).withAlpha (0.12f + wear.nextFloat() * 0.2f));
        g.drawLine (px, py, px + std::cos (la) * len, py + std::sin (la) * len, 0.6f);
    }
    if (corrosion > 0.5f)
    {
        g.setColour (juce::Colour (0xff4f8a84).withAlpha (0.25f * corrosion));
        const float a = wear.nextFloat() * juce::MathConstants<float>::twoPi;
        g.fillEllipse (c.x + std::cos (a) * cap * 0.5f - cap * 0.3f, c.y + std::sin (a) * cap * 0.5f - cap * 0.25f, cap * 0.6f, cap * 0.5f);
    }
    g.setColour (juce::Colours::black.withAlpha (0.7f));
    g.drawEllipse (c.x - cap, c.y - cap, cap * 2.0f, cap * 2.0f, 1.0f);

    // Visare: kort ljus markering på kjolen.
    juce::Path pointer;
    pointer.addRoundedRectangle (-1.1f, -skirt * 0.98f, 2.2f, skirt - cap - 1.0f, 1.0f);
    g.setColour (colours::text.withAlpha (0.8f));
    g.fillPath (pointer, juce::AffineTransform::rotation (angle).translated (c.x, c.y));
}

void LookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool, int, int, int, int, juce::ComboBox&)
{
    const auto r = juce::Rectangle<float> (0.0f, 0.0f, (float) width, (float) height);
    g.setColour (colours::bakelite);
    g.fillRect (r.reduced (1.0f));
    drawEngravedBox (g, r.reduced (0.5f));

    juce::Path arrow;
    const float ax = (float) width - 11.0f, ay = (float) height * 0.5f;
    arrow.addTriangle (ax - 3.5f, ay - 2.0f, ax + 3.5f, ay - 2.0f, ax, ay + 3.0f);
    g.setColour (colours::dimText);
    g.fillPath (arrow);
}

void LookAndFeel::positionComboBoxText (juce::ComboBox& box, juce::Label& label)
{
    label.setBounds (4, 1, box.getWidth() - 18, box.getHeight() - 2);
    label.setFont (getComboBoxFont (box));
}

juce::Font LookAndFeel::getComboBoxFont (juce::ComboBox&)
{
    return panelFont (14.5f).withExtraKerningFactor (0.04f);
}

juce::Font LookAndFeel::getPopupMenuFont()
{
    return panelFont (16.0f, false);
}

juce::Font LookAndFeel::getTextButtonFont (juce::TextButton&, int buttonHeight)
{
    return panelFont (std::min (15.0f, (float) buttonHeight * 0.62f));
}

void LookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool highlighted, bool down)
{
    auto r = b.getLocalBounds().toFloat().reduced (1.0f);
    g.setGradientFill (juce::ColourGradient (down ? colours::metalDark : colours::metal.brighter (highlighted ? 0.18f : 0.08f),
                                             r.getX(), r.getY(),
                                             down ? colours::metal : colours::metalDark, r.getX(), r.getBottom(), false));
    g.fillRect (r);
    // Upphöjd knapp: ljus kant upptill, mörk nedtill (omvänt när den trycks ner).
    g.setColour ((down ? colours::engraveDark : colours::engraveLight).withAlpha (0.8f));
    g.drawLine (r.getX(), r.getY(), r.getRight(), r.getY(), 1.0f);
    g.setColour ((down ? colours::engraveLight : colours::engraveDark).withAlpha (0.9f));
    g.drawLine (r.getX(), r.getBottom(), r.getRight(), r.getBottom(), 1.2f);
    g.setColour (colours::engraveDark);
    g.drawRect (r.expanded (0.5f), 1.0f);

    // Växlingsknappar har en signallampa till vänster. "active" = något pågår (t.ex. sekvensern spelar).
    if (b.getClickingTogglesState())
    {
        const bool lit = b.getToggleState() || (bool) b.getProperties().getWithDefault ("active", false);
        const auto colour = b.getToggleState() ? colours::lampRed : colours::lampAmber;
        const float d = std::min (12.0f, r.getHeight() - 8.0f);
        drawLamp (g, juce::Rectangle<float> (r.getX() + 6.0f, r.getCentreY() - d * 0.5f, d, d), colour,
                  colours::lampRedOff, lit);
    }
}

void LookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& b, bool, bool)
{
    auto r = b.getLocalBounds().reduced (4, 0);
    if (b.getClickingTogglesState())
        r.removeFromLeft (18);   // plats för lampan
    g.setFont (getTextButtonFont (b, b.getHeight()));
    g.setColour (colours::text.withAlpha (b.isEnabled() ? 1.0f : 0.4f));
    g.drawFittedText (b.getButtonText(), r, juce::Justification::centred, 1);
}

juce::Label* LookAndFeel::createSliderTextBox (juce::Slider& slider)
{
    // Sifferrutor (Pulses, Från/Till …): mörkt fönster med ljus text, som resten av panelen.
    auto* l = LookAndFeel_V4::createSliderTextBox (slider);
    l->setColour (juce::Label::backgroundColourId, colours::bakelite);
    l->setColour (juce::Label::textColourId, colours::text);
    l->setColour (juce::Label::outlineColourId, colours::engraveLight.withAlpha (0.7f));
    l->setColour (juce::TextEditor::backgroundColourId, colours::bakelite);
    l->setColour (juce::TextEditor::textColourId, colours::text);
    l->setColour (juce::TextEditor::highlightColourId, colours::lampAmber.withAlpha (0.4f));
    l->setFont (panelFont (14.0f, false));
    return l;
}

void LookAndFeel::drawLabel (juce::Graphics& g, juce::Label& label)
{
    // Sifferrutor i sliders: mörkt nedsänkt fönster med ljus text.
    if (dynamic_cast<juce::Slider*> (label.getParentComponent()) != nullptr)
    {
        const auto r = label.getLocalBounds().toFloat();
        g.setColour (colours::bakelite);
        g.fillRect (r.reduced (0.5f));
        drawEngravedBox (g, r.reduced (0.5f));
        if (! label.isBeingEdited())
        {
            g.setColour (colours::text.withAlpha (label.isEnabled() ? 0.95f : 0.4f));
            g.setFont (panelFont (14.0f, false));
            g.drawFittedText (label.getText(), label.getLocalBounds().reduced (2, 0), juce::Justification::centred, 1);
        }
        return;
    }
    LookAndFeel_V4::drawLabel (g, label);
}

void LookAndFeel::drawPopupMenuBackground (juce::Graphics& g, int width, int height)
{
    g.fillAll (colours::metalDark);
    g.setColour (colours::engraveLight.withAlpha (0.6f));
    g.drawRect (0, 0, width, height, 1);
}

//==============================================================================
Control::Control (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId, const juce::String& text,
                  juce::Colour colour, bool isChoice)
{
    label.setText (text.toUpperCase(), juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setFont (panelFont (12.5f, false).withExtraKerningFactor (0.14f));
    label.setColour (juce::Label::textColourId, colours::text.withAlpha (0.92f));
    label.setInterceptsMouseClicks (false, false);
    addAndMakeVisible (label);

    auto* param = apvts.getParameter (paramId);
    jassert (param != nullptr);

    if (isChoice)
    {
        combo = std::make_unique<juce::ComboBox>();
        if (auto* choice = dynamic_cast<juce::AudioParameterChoice*> (param))
            combo->addItemList (choice->choices, 1);
        else if (dynamic_cast<juce::AudioParameterBool*> (param) != nullptr)
            combo->addItemList ({ "Off", "On" }, 1);
        comboAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, paramId, *combo);
        combo->setTooltip (param->getName (64));
        addAndMakeVisible (*combo);
    }
    else
    {
        slider = std::make_unique<juce::Slider> (juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::NoTextBox);
        slider->setName (paramId);   // ger varje ratt eget, fast slitage
        slider->setColour (juce::Slider::rotarySliderFillColourId, colour);
        slider->setPopupDisplayEnabled (true, true, nullptr, 1500);
        slider->setMouseDragSensitivity (220);
        sliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, paramId, *slider);
        if (auto* ranged = dynamic_cast<juce::RangedAudioParameter*> (param))
            slider->setDoubleClickReturnValue (true, ranged->convertFrom0to1 (ranged->getDefaultValue()));
        slider->setTooltip (param->getName (64) + " (dubbelklick = standard / double-click = default)");
        addAndMakeVisible (*slider);
    }
}

void Control::resized()
{
    auto r = getLocalBounds();
    auto labelArea = r.removeFromBottom (17);
    label.setBounds (labelArea);

    if (slider != nullptr)
    {
        const int size = std::min (r.getWidth(), r.getHeight());
        slider->setBounds (r.withSizeKeepingCentre (size, size));
    }
    else if (combo != nullptr)
    {
        combo->setBounds (r.withSizeKeepingCentre (std::min (r.getWidth() - 4, 104), 24));
    }
}

//==============================================================================
Section::Section (const char* t, juce::Colour c) : title (juce::String::fromUTF8 (t)), colour (c) {}

void Section::add (juce::AudioProcessorValueTreeState& apvts, const juce::String& paramId, const juce::String& label,
                   bool isChoice)
{
    controls.push_back (std::make_unique<Control> (apvts, paramId, label, colour, isChoice));
    addAndMakeVisible (*controls.back());
}

void Section::paint (juce::Graphics& g)
{
    drawSectionFrame (g, getLocalBounds().toFloat().reduced (3.0f), title, note);

    // Grupp (undertonen): tunn ram runt rattarna och en liten rubrik i ramens överkant.
    if (groupFirst >= 0 && groupFirst < (int) controls.size())
    {
        auto box = controls[(size_t) groupFirst]->getBounds().getUnion (controls.back()->getBounds()).toFloat()
                       .expanded (2.0f, 0.0f).withTrimmedTop (-3.0f).withTrimmedBottom (-1.0f);
        g.setColour (juce::Colours::black.withAlpha (0.28f));
        g.fillRoundedRectangle (box, 3.0f);
        g.setColour (colours::hairline.withAlpha (0.45f));
        g.drawRoundedRectangle (box, 3.0f, 1.0f);

        const auto f = panelFont (9.5f, false).withExtraKerningFactor (0.12f);
        const float tw = juce::GlyphArrangement::getStringWidth (f, groupCaption) + 8.0f;
        auto cap = juce::Rectangle<float> (box.getCentreX() - tw * 0.5f, box.getY() - 5.0f, tw, 10.0f);
        g.setColour (colours::metal);
        g.fillRect (cap);
        g.setFont (f);
        g.setColour (colours::text.withAlpha (0.9f));
        g.drawText (groupCaption, cap, juce::Justification::centred);
    }
}

void Section::resized()
{
    auto r = getLocalBounds().reduced (8, 6);
    r.removeFromTop (22);
    if (groupFirst >= 0)
        r.removeFromTop (9);   // plats för gruppens rubrik under sektionsrubriken
    if (controls.empty())
        return;

    const int w = r.getWidth() / (int) controls.size();
    for (auto& c : controls)
        c->setBounds (r.removeFromLeft (w));
}
} // namespace steso::ui

//==============================================================================
StesoLeadEditor::StesoLeadEditor (StesoLeadProcessor& p)
    : AudioProcessorEditor (&p),
      processor (p),
      keyboard (p.keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard),
      seqPanel (p)
{
    using namespace steso;
    setLookAndFeel (&lnf);
    auto& s = p.apvts;

    osc1.add (s, pid::osc1Wave, "Wave", true);
    osc1.add (s, pid::osc1Octave, "Octave");
    osc1.add (s, pid::osc1PW, "PW");
    osc1.add (s, pid::osc1SubDiv, "Sub 1 Div");
    osc1.add (s, pid::mixSubh1, "Sub 1");
    osc1.setGroup (3, juce::String::fromUTF8 ("UNDERTON 1 (SUB 1)"));

    osc2.add (s, pid::osc2Wave, "Wave", true);
    osc2.add (s, pid::osc2Semi, "Semi");
    osc2.add (s, pid::osc2Fine, "Fine");
    osc2.add (s, pid::osc2PW, "PW");
    osc2.add (s, pid::osc2SubDiv, "Sub 2 Div");
    osc2.add (s, pid::mixSubh2, "Sub 2");
    osc2.setGroup (4, juce::String::fromUTF8 ("UNDERTON 2 (SUB 2)"));

    osc3.add (s, pid::osc3Wave, "Wave", true);
    osc3.add (s, pid::osc3Semi, "Semi");
    osc3.add (s, pid::osc3Fine, "Fine");
    osc3.add (s, pid::osc3SubDiv, "Sub 3 Div");
    osc3.add (s, pid::mixSubh3, "Sub 3");
    osc3.setGroup (3, juce::String::fromUTF8 ("UNDERTON 3 (SUB 3)"));

    mixer.add (s, pid::mixOsc1, "Osc 1");
    mixer.add (s, pid::mixOsc2, "Osc 2");
    mixer.add (s, pid::mixOsc3, "Osc 3");
    mixer.add (s, pid::mixSub, "Sub Oct");
    mixer.add (s, pid::mixNoise, "Noise");

    filter.add (s, pid::filterType, "Type", true);
    filter.add (s, pid::cutoff, "Cutoff");
    filter.add (s, pid::resonance, "Reso");
    filter.add (s, pid::drive, "Drive");
    filter.add (s, pid::filterEnv, "Env");
    filter.add (s, pid::keyTrack, "Key");
    filter.add (s, pid::filterVel, "Velocity");
    filter.add (s, pid::feedback, "Feedback");
    filter.add (s, pid::warmth, "Warmth");

    filterEnv.add (s, pid::fAttack, "Attack");
    filterEnv.add (s, pid::fDecay, "Decay");
    filterEnv.add (s, pid::fSustain, "Sustain");
    filterEnv.add (s, pid::fRelease, "Release");

    ampEnv.add (s, pid::aAttack, "Attack");
    ampEnv.add (s, pid::aDecay, "Decay");
    ampEnv.add (s, pid::aSustain, "Sustain");
    ampEnv.add (s, pid::aRelease, "Release");
    ampEnv.add (s, pid::ampVel, "Velocity");

    lfo.add (s, pid::lfoWave, "Wave", true);
    lfo.add (s, pid::lfoRate, "Rate");
    lfo.add (s, pid::lfoFade, "Fade In");
    lfo.add (s, pid::lfoPitch, "Pitch");
    lfo.add (s, pid::lfoFilter, "Filter");
    lfo.add (s, pid::lfoPW, "PW");

    voice.add (s, pid::voiceMode, "Mode", true);
    voice.add (s, pid::glide, "Glide");
    voice.add (s, pid::bendRange, "Bend");
    voice.add (s, pid::drift, "Drift");
    voice.add (s, pid::spread, "Spread");
    voice.add (s, pid::unison, "Unison");
    voice.add (s, pid::uniDetune, "Detune");

    fx.add (s, pid::chorus, "Chorus", true);
    fx.add (s, pid::delayTime, "Time");
    fx.add (s, pid::delayFb, "Feedback");
    fx.add (s, pid::delayMix, "Delay");
    fx.add (s, pid::reverbMix, "Reverb");

    output.add (s, pid::master, "Volume");

    for (auto* c : std::initializer_list<juce::Component*> { &osc1, &osc2, &osc3, &mixer, &filter, &filterEnv, &ampEnv,
                                                             &lfo, &voice, &fx, &output })
        content.addAndMakeVisible (c);

    rebuildPresetList();
    presetBox.setTextWhenNothingSelected (juce::String::fromUTF8 ("Förinställning (Preset)"));
    presetBox.onChange = [this]
    {
        const int id = presetBox.getSelectedId();
        if (id >= userIdBase)
            selectEntry (-(id - userIdBase + 1));
        else if (id > 0)
            selectEntry (id - 1);
    };
    content.addAndMakeVisible (presetBox);

    prevButton.onClick = [this] { stepPreset (-1); };
    nextButton.onClick = [this] { stepPreset (1); };
    content.addAndMakeVisible (prevButton);
    content.addAndMakeVisible (nextButton);

    saveButton.setTooltip (juce::String::fromUTF8 ("Spara inställningarna som egen förinställning (save as user preset): ")
                           + StesoLeadProcessor::getUserPresetFolder().getFullPathName());
    saveButton.onClick = [this] { savePresetDialog(); };
    deleteButton.setTooltip (juce::String::fromUTF8 ("Ta bort den valda egna förinställningen (delete the selected user preset)"));
    deleteButton.onClick = [this] { deletePresetDialog(); };
    content.addAndMakeVisible (saveButton);
    content.addAndMakeVisible (deleteButton);


    keyboard.setAvailableRange (24, 108);
    keyboard.setLowestVisibleKey (36);
    keyboard.setKeyWidth (22.0f);
    // Gulnade tangenter.
    keyboard.setColour (juce::MidiKeyboardComponent::whiteNoteColourId, juce::Colour (0xffaaa597));
    keyboard.setColour (juce::MidiKeyboardComponent::blackNoteColourId, juce::Colour (0xff141412));
    keyboard.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, steso::ui::colours::lampRed.withAlpha (0.55f));
    keyboard.setColour (juce::MidiKeyboardComponent::mouseOverKeyOverlayColourId, steso::ui::colours::lampAmber.withAlpha (0.25f));
    keyboard.setColour (juce::MidiKeyboardComponent::shadowColourId, juce::Colours::black.withAlpha (0.35f));
    keyboard.setColour (juce::MidiKeyboardComponent::keySeparatorLineColourId, juce::Colour (0xff6e6655));
    keyboard.setColour (juce::MidiKeyboardComponent::textLabelColourId, juce::Colour (0xff5d5647));
    keyboard.setColour (juce::MidiKeyboardComponent::upDownButtonBackgroundColourId, steso::ui::colours::metal);
    keyboard.setColour (juce::MidiKeyboardComponent::upDownButtonArrowColourId, steso::ui::colours::text);
    keyboard.setWantsKeyboardFocus (false);
    content.addAndMakeVisible (keyboard);
    content.addAndMakeVisible (seqPanel);
    // Panelerna skapades före setLookAndFeel: låt alla barn (t.ex. sifferrutor) byggas om med vårt skin.
    sendLookAndFeelChange();

    addAndMakeVisible (content);
    // Utfällda delar sparas i projektet (inte som parametrar).
    showSeq = (bool) processor.apvts.state.getProperty ("uiShowSeq", false);
    showKeys = (bool) processor.apvts.state.getProperty ("uiShowKeys", false);
    for (auto* t : { &seqToggle, &keysToggle })
    {
        t->setClickingTogglesState (true);
        content.addAndMakeVisible (*t);
    }
    seqToggle.setToggleState (showSeq, juce::dontSendNotification);
    keysToggle.setToggleState (showKeys, juce::dontSendNotification);
    seqToggle.setTooltip (juce::String::fromUTF8 ("Visa/dölj stegsekvenseraren (show/hide step sequencer). Lampan lyser gult när den spelar."));
    keysToggle.setTooltip (juce::String::fromUTF8 ("Visa/dölj klaviaturen (show/hide keyboard)"));
    seqToggle.onClick = [this] { setPanelsVisible (seqToggle.getToggleState(), showKeys); };
    keysToggle.onClick = [this] { setPanelsVisible (showSeq, keysToggle.getToggleState()); };

    setResizable (true, true);
    applyPanelSize (1.0f);

    refreshPresetBox();
    startTimerHz (30);
}

StesoLeadEditor::~StesoLeadEditor()
{
    setLookAndFeel (nullptr);
}

int StesoLeadEditor::currentBaseHeight() const
{
    return synthH + (showSeq ? unitGap + seqH : 0) + (showKeys ? unitGap + keysH : 0);
}

void StesoLeadEditor::setPanelsVisible (bool sequencer, bool keys)
{
    showSeq = sequencer;
    showKeys = keys;
    processor.apvts.state.setProperty ("uiShowSeq", showSeq, nullptr);
    processor.apvts.state.setProperty ("uiShowKeys", showKeys, nullptr);
    seqToggle.setToggleState (showSeq, juce::dontSendNotification);
    keysToggle.setToggleState (showKeys, juce::dontSendNotification);
    applyPanelSize (getWidth() > 0 ? (float) getWidth() / (float) baseWidth : 1.0f);
}

void StesoLeadEditor::applyPanelSize (float scale)
{
    const int h = currentBaseHeight();
    seqPanel.setVisible (showSeq);
    keyboard.setVisible (showKeys);
    content.setSize (baseWidth, h);

    // Nya gränser och proportioner för den nya höjden innan storleken ändras.
    setResizeLimits (baseWidth * 6 / 10, h * 6 / 10, baseWidth * 2, h * 2);
    getConstrainer()->setFixedAspectRatio ((double) baseWidth / (double) h);
    setSize (juce::roundToInt ((float) baseWidth * scale), juce::roundToInt ((float) h * scale));
    resized();
    repaint();
}

void StesoLeadEditor::paintOverChildren (juce::Graphics& g)
{
    using namespace steso::ui;
    g.addTransform (content.getTransform());
    if (grimeOverlay.isNull())
        grimeOverlay = makeGrimeOverlay (baseWidth, baseHeight);

    // Smuts och korn över allt, sedan punktljuset – per rackenhet, så att varje enhet har sin ljuskägla.
    const int h = currentBaseHeight();
    g.setOpacity (0.4f);
    g.drawImage (grimeOverlay, 0, 0, baseWidth, h, 0, 0, baseWidth, h);
    g.setOpacity (1.0f);

    auto light = [&] (int y, int unitH, float strength)
    {
        const auto area = juce::Rectangle<float> (0.0f, (float) y, (float) baseWidth, (float) unitH);
        juce::Graphics::ScopedSaveState save (g);
        g.reduceClipRegion (area.toNearestInt());
        g.setOpacity (strength);
        drawSpotLight (g, area, { (float) baseWidth * 0.46f, (float) y + (float) unitH * 0.32f });
    };
    int y = 0;
    // Svagt över kontrollerna – mörkret ligger i bakgrunden (paint), så att allt går att läsa.
    light (y, synthH, 0.1f);
    y += synthH;
    if (showSeq)  { y += unitGap; light (y, seqH, 0.1f);  y += seqH; }
    if (showKeys) { y += unitGap; light (y, keysH, 0.08f); }
}

void StesoLeadEditor::paint (juce::Graphics& g)
{
    using namespace steso::ui;
    g.fillAll (juce::Colour (0xff0c0c0b));   // racket bakom enheterna

    g.addTransform (content.getTransform());
    if (panelTexture.isNull())
        panelTexture = makePanelTexture (innerW, baseHeight);

    // Rita en rackenhet: plåt med textur + rackfästen på båda sidor.
    auto drawUnit = [&] (int y, int h)
    {
        g.drawImage (panelTexture, earW, y, innerW, h, 0, y, innerW, h);
        g.setColour (colours::engraveDark);
        g.drawRect (juce::Rectangle<int> (earW, y, innerW, h), 1);
        drawRackEar (g, juce::Rectangle<float> (0.0f, (float) y, (float) earW, (float) h));
        drawRackEar (g, juce::Rectangle<float> ((float) (earW + innerW), (float) y, (float) earW, (float) h));
    };

    int y = 0;
    drawUnit (y, synthH);
    y += synthH;
    if (showSeq)  { y += unitGap; drawUnit (y, seqH);  y += seqH; }
    if (showKeys) { y += unitGap; drawUnit (y, keysH); y += keysH; }

    // Punktljuset fullt på plåten (bakom kontrollerna): mörkt i kanterna utan att texten försvinner.
    {
        auto spot = [&] (int uy, int uh)
        {
            juce::Graphics::ScopedSaveState save (g);
            const auto area = juce::Rectangle<float> (0.0f, (float) uy, (float) baseWidth, (float) uh);
            g.reduceClipRegion (area.toNearestInt());
            drawSpotLight (g, area, { (float) baseWidth * 0.46f, (float) uy + (float) uh * 0.32f });
        };
        int uy = 0;
        spot (uy, synthH);
        uy += synthH;
        if (showSeq)  { uy += unitGap; spot (uy, seqH);  uy += seqH; }
        if (showKeys) { uy += unitGap; spot (uy, keysH); }
    }

    // Logga: präglad i plåten, nästan ton-i-ton (skugga uppåt-vänster, ljuskant nedåt-höger).
    auto header = juce::Rectangle<float> ((float) earW + 20.0f, 8.0f, 600.0f, 44.0f);
    const auto title = panelFont (30.0f).withExtraKerningFactor (0.22f);
    g.setFont (title);
    g.setColour (juce::Colours::black.withAlpha (0.7f));
    g.drawText ("BAKELIT", header.translated (-0.8f, -0.8f), juce::Justification::centredLeft);
    g.setColour (juce::Colours::white.withAlpha (0.07f));
    g.drawText ("BAKELIT", header.translated (0.8f, 0.9f), juce::Justification::centredLeft);
    g.setColour (colours::metal.brighter (0.55f));
    g.drawText ("BAKELIT", header, juce::Justification::centredLeft);

    const float w = juce::GlyphArrangement::getStringWidth (title, "BAKELIT");
    auto sub = header.withTrimmedLeft (w + 14.0f).translated (0.0f, 4.0f);
    g.setFont (panelFont (11.5f, false).withExtraKerningFactor (0.2f));
    g.setColour (colours::dimText.withAlpha (0.75f));
    g.drawText (juce::String::fromUTF8 ("FALK INSTRUMENTS  ·  v" STESO_VERSION), sub, juce::Justification::centredLeft);
}

void StesoLeadEditor::resized()
{
    const float scale = (float) getWidth() / (float) baseWidth;
    content.setTransform (juce::AffineTransform::scale (scale));

    // --- Synthenheten
    auto r = juce::Rectangle<int> (earW, 0, innerW, synthH).reduced (12, 8);

    auto header = r.removeFromTop (46);
    header.removeFromLeft (404);
    nextButton.setBounds (header.removeFromRight (34).reduced (0, 8));
    header.removeFromRight (4);
    presetBox.setBounds (header.removeFromRight (206).reduced (0, 8));
    header.removeFromRight (4);
    prevButton.setBounds (header.removeFromRight (34).reduced (0, 8));
    header.removeFromRight (8);
    deleteButton.setBounds (header.removeFromRight (96).reduced (0, 8));
    header.removeFromRight (4);
    saveButton.setBounds (header.removeFromRight (84).reduced (0, 8));
    header.removeFromRight (8);
    keysToggle.setBounds (header.removeFromRight (94).reduced (0, 8));
    header.removeFromRight (4);
    seqToggle.setBounds (header.removeFromRight (94).reduced (0, 8));

    const int rowH = r.getHeight() / 4;
    const int unit = r.getWidth() / 16;   // 16 kolumner totalt per rad

    auto row1 = r.removeFromTop (rowH);
    {
        // Bredd efter antal rattar: OSC 1 (5), OSC 2 (6), OSC 3 (5), Nivåer (5).
        const int w = row1.getWidth();
        osc1.setBounds (row1.removeFromLeft (w * 5 / 21));
        osc2.setBounds (row1.removeFromLeft (w * 6 / 21));
        osc3.setBounds (row1.removeFromLeft (w * 5 / 21));
        mixer.setBounds (row1);
    }

    auto row2 = r.removeFromTop (rowH);
    filter.setBounds (row2.removeFromLeft (unit * 11));
    filterEnv.setBounds (row2);

    auto row3 = r.removeFromTop (rowH);
    ampEnv.setBounds (row3.removeFromLeft (unit * 7));
    lfo.setBounds (row3);

    auto row4 = r;
    voice.setBounds (row4.removeFromLeft (unit * 8));
    fx.setBounds (row4.removeFromLeft (unit * 6));
    output.setBounds (row4);

    // --- Sekvenser- och klaviaturenheterna under
    int y = synthH;
    if (showSeq)
    {
        y += unitGap;
        seqPanel.setBounds (juce::Rectangle<int> (earW, y, innerW, seqH).reduced (12, 6));
        y += seqH;
    }
    if (showKeys)
    {
        y += unitGap;
        keyboard.setBounds (juce::Rectangle<int> (earW, y, innerW, keysH).reduced (14, 8));
    }
}

void StesoLeadEditor::timerCallback()
{
    const int counter = processor.programChangeCounter.load();
    if (counter != lastProgramCounter)
    {
        lastProgramCounter = counter;
        refreshPresetBox();
        seqPanel.bindState();
    }

    if (showSeq)
        seqPanel.refresh();

    // Lampan på Sekv.-knappen lyser gult när sekvensern spelar men är infälld.
    const bool seqActive = ! showSeq && processor.getCurrentStep() >= 0;
    if ((bool) seqToggle.getProperties().getWithDefault ("active", false) != seqActive)
    {
        seqToggle.getProperties().set ("active", seqActive);
        seqToggle.repaint();
    }

    const int v = processor.getActiveVoiceCount();
    if (v != lastVoices)
    {
        lastVoices = v;
        output.setHeaderNote (juce::String::fromUTF8 ("STÄMMOR ") + juce::String (v) + "/"
                              + juce::String (steso::dsp::SynthEngine::maxVoices));
    }
}

void StesoLeadEditor::rebuildPresetList()
{
    presetBox.clear (juce::dontSendNotification);
    presetOrder.clear();

    // Egna presets överst.
    userFiles = StesoLeadProcessor::getUserPresetFiles();
    if (! userFiles.isEmpty())
    {
        presetBox.addSectionHeading ("Egna (User)");
        for (int i = 0; i < userFiles.size(); ++i)
        {
            presetBox.addItem (userFiles[i].getFileNameWithoutExtension(), userIdBase + i);
            presetOrder.push_back (-(i + 1));
        }
    }

    // Fabrikspresets grupperade per kategori (i den ordning kategorin först förekommer).
    // ID = programnummer + 1, så ordningen i listan påverkar inte sparade projekt.
    const auto& presets = steso::getFactoryPresets();
    juce::StringArray categories;
    for (const auto& pr : presets)
        categories.addIfNotAlreadyThere (pr.category);
    for (const auto& category : categories)
    {
        presetBox.addSectionHeading (category);
        for (size_t i = 0; i < presets.size(); ++i)
            if (presets[i].category == category)
            {
                presetBox.addItem (presets[i].name, (int) i + 1);
                presetOrder.push_back ((int) i);
            }
    }
}

int StesoLeadEditor::currentUserIndex() const
{
    const auto name = processor.getUserPresetName();
    if (name.isEmpty())
        return -1;
    const auto legal = juce::File::createLegalFileName (name);
    for (int i = 0; i < userFiles.size(); ++i)
        if (userFiles[i].getFileNameWithoutExtension() == legal)
            return i;
    return -1;
}

void StesoLeadEditor::selectEntry (int entry)
{
    if (entry >= 0)
    {
        if (entry != processor.getCurrentProgram() || processor.getUserPresetName().isNotEmpty())
            processor.setCurrentProgram (entry);
    }
    else
    {
        const int index = -entry - 1;
        if (juce::isPositiveAndBelow (index, userFiles.size()))
            processor.loadUserPreset (userFiles[index]);
    }
    refreshPresetBox();
}

void StesoLeadEditor::refreshPresetBox()
{
    const int userIndex = currentUserIndex();
    if (userIndex >= 0)
        presetBox.setSelectedId (userIdBase + userIndex, juce::dontSendNotification);
    else if (processor.getUserPresetName().isNotEmpty())
        presetBox.setText (processor.getUserPresetName(), juce::dontSendNotification);   // filen finns inte längre
    else
        presetBox.setSelectedId (processor.getCurrentProgram() + 1, juce::dontSendNotification);

    deleteButton.setEnabled (userIndex >= 0);
}

void StesoLeadEditor::stepPreset (int delta)
{
    // Bläddra i samma ordning som listan visar (egna först, sedan per kategori).
    const int n = (int) presetOrder.size();
    if (n == 0)
        return;
    const int userIndex = currentUserIndex();
    const int current = userIndex >= 0 ? -(userIndex + 1) : processor.getCurrentProgram();
    const auto it = std::find (presetOrder.begin(), presetOrder.end(), current);
    const int pos = it == presetOrder.end() ? 0 : (int) (it - presetOrder.begin());
    selectEntry (presetOrder[(size_t) ((pos + delta + n) % n)]);
}

void StesoLeadEditor::savePresetDialog()
{
    const auto folder = StesoLeadProcessor::getUserPresetFolder();
    auto suggested = processor.getUserPresetName();
    if (suggested.isEmpty())
        suggested = processor.getProgramName (processor.getCurrentProgram()) + " 2";

    auto* window = new juce::AlertWindow (juce::String::fromUTF8 ("Spara förinställning (Save preset)"),
                                          juce::String::fromUTF8 ("Namn (name). Sparas i (saved in):\n") + folder.getFullPathName(),
                                          juce::MessageBoxIconType::NoIcon, this);
    window->setLookAndFeel (&lnf);
    window->addTextEditor ("name", suggested);
    window->addButton ("Spara (Save)", 1, juce::KeyPress (juce::KeyPress::returnKey));
    window->addButton ("Avbryt (Cancel)", 0, juce::KeyPress (juce::KeyPress::escapeKey));

    juce::Component::SafePointer<StesoLeadEditor> safe (this);
    window->enterModalState (true, juce::ModalCallbackFunction::create ([safe, window] (int result)
    {
        if (result != 1 || safe == nullptr)
            return;

        const auto name = window->getTextEditorContents ("name").trim();
        if (name.isEmpty())
            return;

        auto doSave = [safe, name]
        {
            if (safe == nullptr)
                return;
            const auto file = safe->processor.saveUserPreset (name);
            if (file == juce::File())
            {
                juce::AlertWindow::showAsync (juce::MessageBoxOptions()
                                                  .withIconType (juce::MessageBoxIconType::WarningIcon)
                                                  .withTitle ("Kunde inte spara (Could not save)")
                                                  .withMessage (StesoLeadProcessor::getUserPresetFolder().getFullPathName())
                                                  .withButton ("OK"), nullptr);
                return;
            }
            safe->rebuildPresetList();
            safe->refreshPresetBox();
        };

        const auto target = StesoLeadProcessor::getUserPresetFolder()
                                .getChildFile (juce::File::createLegalFileName (name) + StesoLeadProcessor::userPresetExtension);
        if (target.existsAsFile())
        {
            juce::AlertWindow::showAsync (juce::MessageBoxOptions()
                                              .withIconType (juce::MessageBoxIconType::QuestionIcon)
                                              .withTitle (juce::String::fromUTF8 ("Skriva över? (Overwrite?)"))
                                              .withMessage ("\"" + name + juce::String::fromUTF8 ("\" finns redan (already exists)."))
                                              .withButton (juce::String::fromUTF8 ("Skriv över (Overwrite)"))
                                              .withButton ("Avbryt (Cancel)"),
                                          [doSave] (int r) { if (r == 1) doSave(); });
        }
        else
        {
            doSave();
        }
    }), true);
}

void StesoLeadEditor::deletePresetDialog()
{
    const int index = currentUserIndex();
    if (index < 0)
        return;

    const auto file = userFiles[index];
    juce::Component::SafePointer<StesoLeadEditor> safe (this);
    juce::AlertWindow::showAsync (juce::MessageBoxOptions()
                                      .withIconType (juce::MessageBoxIconType::QuestionIcon)
                                      .withTitle (juce::String::fromUTF8 ("Ta bort förinställning? (Delete preset?)"))
                                      .withMessage ("\"" + file.getFileNameWithoutExtension() + "\"")
                                      .withButton (juce::String::fromUTF8 ("Ta bort (Delete)"))
                                      .withButton ("Avbryt (Cancel)"),
                                  [safe, file] (int r)
    {
        if (r != 1 || safe == nullptr)
            return;
        file.moveToTrash();
        // Ljudet ligger kvar oförändrat, men är inte längre kopplat till en fil.
        safe->processor.forgetUserPreset();
        safe->rebuildPresetList();
        safe->refreshPresetBox();
    });
}
