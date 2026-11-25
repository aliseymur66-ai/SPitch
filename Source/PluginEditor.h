#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

namespace SC
{
    inline const juce::Colour bg0 (0xff050b18), panel (0xff0c1830), gold (0xffe8a850), goldHi (0xfff7cc7f),
                              goldDim (0xff8d6b36), text (0xffeadcb8), dim (0xff98a4bd), teal (0xff35c8ea),
                              track (0xff1c2a48), green (0xff7fdc7a);
    inline const juce::String kArtistName = "Seymur Ali";   // başlıqdakı imza - istədiyiniz kimi dəyişin
}

//==============================================================================  ikonlar
inline void drawToolIcon(juce::Graphics& g, int t, juce::Rectangle<float> r)
{
    juce::PathStrokeType st(1.8f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);
    juce::Path p;
    const auto c = r.getCentre();
    const float w = r.getWidth(), h = r.getHeight();
    switch (t)
    {
        case 0: case 1:   // undo / redo
        {
            const float s = (t == 0) ? 1.0f : -1.0f;
            p.startNewSubPath(c.x + s * w * 0.35f, c.y + h * 0.35f);
            p.quadraticTo(c.x + s * w * 0.45f, c.y - h * 0.25f, c.x - s * w * 0.2f, c.y - h * 0.2f);
            g.strokePath(p, st);
            juce::Path a;
            a.startNewSubPath(c.x - s * w * 0.05f, c.y - h * 0.4f);
            a.lineTo(c.x - s * w * 0.25f, c.y - h * 0.2f);
            a.lineTo(c.x - s * w * 0.05f, c.y);
            g.strokePath(a, st);
            break;
        }
        case 2:           // qələm
            g.drawLine(c.x - w * 0.3f, c.y + h * 0.3f, c.x + w * 0.25f, c.y - h * 0.25f, 3.0f);
            g.drawLine(c.x - w * 0.35f, c.y + h * 0.4f, c.x - w * 0.1f, c.y + h * 0.4f, 1.5f);
            break;
        case 3:           // seçim (punktir düzbucaqlı)
        {
            p.addRectangle(r.reduced(w * 0.1f));
            juce::Path d; const float dl[] = { 3.0f, 3.0f };
            juce::PathStrokeType(1.6f).createDashedStroke(d, p, dl, 2);
            g.fillPath(d);
            break;
        }
        case 4:           // maqnit
            p.startNewSubPath(c.x - w * 0.3f, c.y - h * 0.35f);
            p.lineTo(c.x - w * 0.3f, c.y + h * 0.05f);
            p.quadraticTo(c.x - w * 0.3f, c.y + h * 0.4f, c.x, c.y + h * 0.4f);
            p.quadraticTo(c.x + w * 0.3f, c.y + h * 0.4f, c.x + w * 0.3f, c.y + h * 0.05f);
            p.lineTo(c.x + w * 0.3f, c.y - h * 0.35f);
            g.strokePath(p, juce::PathStrokeType(3.0f));
            break;
        case 5:           // dalğa
            for (int i = 0; i <= 16; ++i)
            {
                const float x = r.getX() + w * static_cast<float>(i) / 16.0f;
                const float y = c.y + std::sin(static_cast<float>(i) * 0.9f) * h * 0.3f * (0.4f + 0.6f * std::sin(static_cast<float>(i) * 0.2f + 0.5f));
                if (i == 0) p.startNewSubPath(x, y); else p.lineTo(x, y);
            }
            g.strokePath(p, st);
            break;
        case 6:           // dişli (ayarlar)
        {
            const float rr = std::min(w, h) * 0.5f;
            p.addStar(c, 8, rr * 0.7f, rr, 0.0f);
            g.fillPath(p);
            g.setColour(SC::bg0);
            g.fillEllipse(c.x - rr * 0.3f, c.y - rr * 0.3f, rr * 0.6f, rr * 0.6f);
            break;
        }
        case 7:           // kiçilt
            g.drawLine(c.x - w * 0.35f, c.y, c.x + w * 0.35f, c.y, 1.8f);
            break;
        case 8:           // bağla
            g.drawLine(c.x - w * 0.3f, c.y - h * 0.3f, c.x + w * 0.3f, c.y + h * 0.3f, 1.8f);
            g.drawLine(c.x - w * 0.3f, c.y + h * 0.3f, c.x + w * 0.3f, c.y - h * 0.3f, 1.8f);
            break;
        case 9: case 10:  // oxlar
        {
            const float s = (t == 9) ? -1.0f : 1.0f;
            p.startNewSubPath(c.x - s * w * 0.15f, c.y - h * 0.3f);
            p.lineTo(c.x + s * w * 0.15f, c.y);
            p.lineTo(c.x - s * w * 0.15f, c.y + h * 0.3f);
            g.strokePath(p, st);
            break;
        }
        case 11:          // saxla (disket)
            g.drawRoundedRectangle(r.reduced(w * 0.12f), 2.0f, 1.8f);
            g.drawRect(c.x - w * 0.2f, r.getY() + h * 0.12f, w * 0.4f, h * 0.25f, 1.4f);
            g.drawRect(c.x - w * 0.22f, c.y + h * 0.05f, w * 0.44f, h * 0.3f, 1.4f);
            break;
        default: break;
    }
}

inline void drawNavIcon(juce::Graphics& g, int i, juce::Rectangle<float> r)
{
    const auto c = r.getCentre();
    const float s = std::min(r.getWidth(), r.getHeight()) * 0.5f;
    juce::PathStrokeType st(1.6f);
    juce::Path p;
    switch (i)
    {
        case 0: case 3:
            for (int k = 0; k <= 12; ++k)
            {
                const float x = c.x - s + 2.0f * s * static_cast<float>(k) / 12.0f;
                const float y = c.y + std::sin(static_cast<float>(k) * 1.1f) * s * 0.55f;
                if (k == 0) p.startNewSubPath(x, y); else p.lineTo(x, y);
            }
            g.strokePath(p, st);
            break;
        case 1: g.drawEllipse(c.x - s, c.y - s, 2 * s, 2 * s, 1.6f); g.fillEllipse(c.x - 2, c.y - 2, 4, 4); break;
        case 2: g.drawEllipse(c.x - s, c.y - s * 0.5f, s, s, 1.5f); g.drawEllipse(c.x, c.y - s * 0.5f, s, s, 1.5f); break;
        case 4: g.drawLine(c.x - s * 0.4f, c.y - s, c.x - s * 0.4f, c.y, 1.6f); g.drawLine(c.x + s * 0.4f, c.y - s, c.x + s * 0.4f, c.y, 1.6f);
                g.drawLine(c.x, c.y, c.x, c.y + s, 1.6f); break;
        case 5: g.drawEllipse(c.x - s, c.y - s, 2 * s, 2 * s, 1.6f); g.drawEllipse(c.x - s * 0.5f, c.y - s * 0.5f, s, s, 1.4f); break;
        case 6: for (int k = 0; k < 3; ++k) g.drawLine(c.x - s * 0.7f + k * s * 0.7f, c.y + s * 0.8f, c.x - s * 0.7f + k * s * 0.7f, c.y - s * (0.3f + 0.35f * static_cast<float>(k % 2 == 0 ? 1 : 2)), 2.0f); break;
        case 7: p.addStar(c, 4, s * 0.3f, s, 0.0f); g.strokePath(p, st); break;
        default: p.addRectangle(c.x - s * 0.7f, c.y - s * 0.7f, s * 1.4f, s * 1.4f);
                 p.applyTransform(juce::AffineTransform::rotation(0.785f, c.x, c.y)); g.strokePath(p, st); break;
    }
}

//==============================================================================  LookAndFeel
class SPitchLAF : public juce::LookAndFeel_V4
{
public:
    SPitchLAF()
    {
        setColour(juce::PopupMenu::backgroundColourId, SC::panel);
        setColour(juce::PopupMenu::textColourId, SC::text);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, SC::goldDim);
        setColour(juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
        setColour(juce::ComboBox::textColourId, SC::text);
        setColour(juce::AlertWindow::backgroundColourId, SC::panel);
        setColour(juce::AlertWindow::textColourId, SC::text);
        setColour(juce::TextButton::buttonColourId, SC::goldDim);
        setColour(juce::TextEditor::backgroundColourId, SC::bg0);
        setColour(juce::TextEditor::textColourId, SC::text);
    }

    void drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h, float pos, float startA, float endA, juce::Slider& s) override
    {
        auto b = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y), static_cast<float>(w), static_cast<float>(h)).reduced(3.0f);
        const float r = std::min(b.getWidth(), b.getHeight()) * 0.5f;
        const auto c = b.getCentre();
        const float tw = std::max(4.0f, r * 0.14f);
        const float a = startA + pos * (endA - startA);

        juce::Path track; track.addCentredArc(c.x, c.y, r - tw * 0.5f, r - tw * 0.5f, 0.0f, startA, endA, true);
        g.setColour(SC::track);
        g.strokePath(track, juce::PathStrokeType(tw, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        juce::Path val; val.addCentredArc(c.x, c.y, r - tw * 0.5f, r - tw * 0.5f, 0.0f, startA, a, true);
        g.setColour(SC::gold.withAlpha(0.25f));
        g.strokePath(val, juce::PathStrokeType(tw + 5.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour(SC::gold);
        g.strokePath(val, juce::PathStrokeType(tw, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        const float ri = r - tw - 3.0f;
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff1b2745), c.x, c.y - ri, juce::Colour(0xff0a1226), c.x, c.y + ri, false));
        g.fillEllipse(c.x - ri, c.y - ri, ri * 2, ri * 2);

        const auto kind = s.getProperties()["kind"].toString();
        if (kind == "pct")
        {
            g.setColour(SC::text);
            g.setFont(juce::Font(ri * 0.62f, juce::Font::bold));
            g.drawText(juce::String(juce::roundToInt(static_cast<float>(s.getValue()) * 100.0f)) + "%",
                       juce::Rectangle<float>(c.x - ri, c.y - ri * 0.4f, ri * 2, ri * 0.8f), juce::Justification::centred, false);
        }
        else
        {
            const float L = ri * 0.85f;
            g.setColour(SC::goldHi);
            g.drawLine(c.x + std::sin(a) * ri * 0.25f, c.y - std::cos(a) * ri * 0.25f,
                       c.x + std::sin(a) * L, c.y - std::cos(a) * L, 2.2f);
        }
    }

    void drawLinearSlider(juce::Graphics& g, int x, int y, int w, int h, float pos, float, float, juce::Slider::SliderStyle, juce::Slider&) override
    {
        const float cy = static_cast<float>(y) + static_cast<float>(h) * 0.5f;
        juce::Rectangle<float> tr(static_cast<float>(x) + 2.0f, cy - 2.0f, static_cast<float>(w) - 4.0f, 4.0f);
        g.setColour(SC::track); g.fillRoundedRectangle(tr, 2.0f);
        g.setColour(SC::gold); g.fillRoundedRectangle(tr.withRight(pos), 2.0f);
        g.setColour(SC::gold.withAlpha(0.3f)); g.fillEllipse(pos - 9.0f, cy - 9.0f, 18.0f, 18.0f);
        g.setColour(SC::goldHi); g.fillEllipse(pos - 6.0f, cy - 6.0f, 12.0f, 12.0f);
    }

    void drawComboBox(juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox&) override
    {
        juce::Rectangle<float> r(0.5f, 0.5f, static_cast<float>(w) - 1.0f, static_cast<float>(h) - 1.0f);
        g.setColour(juce::Colour(0xff0a1428)); g.fillRoundedRectangle(r, 8.0f);
        g.setColour(SC::goldDim); g.drawRoundedRectangle(r, 8.0f, 1.2f);
        juce::Path p; const float cx = static_cast<float>(w) - 15.0f, cy = static_cast<float>(h) * 0.5f;
        p.startNewSubPath(cx - 4, cy - 2); p.lineTo(cx, cy + 2); p.lineTo(cx + 4, cy - 2);
        g.setColour(SC::gold); g.strokePath(p, juce::PathStrokeType(1.6f));
    }
    juce::Font getComboBoxFont(juce::ComboBox&) override { return juce::Font(15.0f); }
    void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override
    {
        label.setBounds(8, 1, box.getWidth() - 26, box.getHeight() - 2);
        label.setFont(getComboBoxFont(box));
    }

    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& b, bool, bool) override
    {
        auto r = b.getLocalBounds().toFloat().reduced(1.0f);
        const bool on = b.getToggleState();
        g.setColour(on ? SC::goldDim : SC::track); g.fillRoundedRectangle(r, r.getHeight() / 2);
        g.setColour(SC::gold.withAlpha(0.8f)); g.drawRoundedRectangle(r, r.getHeight() / 2, 1.0f);
        const float d = r.getHeight() - 6.0f;
        g.setColour(on ? SC::goldHi : juce::Colour(0xff6b7691));
        g.fillEllipse(on ? r.getRight() - d - 3.0f : r.getX() + 3.0f, r.getY() + 3.0f, d, d);
    }
};

//==============================================================================  düymələr
class IconButton : public juce::Button
{
public:
    explicit IconButton(int t) : juce::Button("icon"), type(t) {}
    void paintButton(juce::Graphics& g, bool over, bool down) override
    {
        auto r = getLocalBounds().toFloat();
        const bool on = getToggleState();
        if (on || over) { g.setColour(SC::gold.withAlpha(on ? 0.25f : 0.12f)); g.fillRoundedRectangle(r.reduced(2.0f), 6.0f); }
        g.setColour(on ? SC::goldHi : SC::gold.withAlpha(down ? 1.0f : 0.85f));
        drawToolIcon(g, type, r.reduced(r.getWidth() * 0.25f));
    }
    int type;
};

class NavButton : public juce::Button
{
public:
    NavButton(const juce::String& t, int i) : juce::Button(t), idx(i) {}
    void paintButton(juce::Graphics& g, bool over, bool) override
    {
        auto r = getLocalBounds().toFloat().reduced(1.0f);
        if (selected) { g.setColour(SC::gold.withAlpha(0.12f)); g.fillRoundedRectangle(r, 8.0f);
                        g.setColour(SC::gold.withAlpha(0.7f)); g.drawRoundedRectangle(r, 8.0f, 1.2f); }
        else if (over) { g.setColour(SC::gold.withAlpha(0.08f)); g.fillRoundedRectangle(r, 8.0f); }
        g.setColour(selected ? SC::goldHi : SC::gold.withAlpha(0.75f));
        drawNavIcon(g, idx, { r.getX() + 10.0f, r.getY() + 5.0f, 22.0f, r.getHeight() - 10.0f });
        g.setColour(selected ? SC::text : SC::dim);
        g.setFont(juce::Font(16.0f));
        g.drawText(getName(), r.withTrimmedLeft(44.0f), juce::Justification::centredLeft, false);
    }
    bool selected = false;
    int idx;
};

class FlatToggle : public juce::Button
{
public:
    FlatToggle() : juce::Button("flat") {}
    std::function<void(juce::Graphics&, bool, juce::Rectangle<int>)> painter;
    void paintButton(juce::Graphics& g, bool, bool) override { if (painter) painter(g, getToggleState(), getLocalBounds()); }
};

//==============================================================================  pitch qrafiki
class PitchGraph : public juce::Component
{
public:
    struct Pt { float in = 0.0f, tgt = 0.0f; bool voiced = false; };
    static constexpr int N = 320;

    void push(float inMidi, float tgtMidi, bool voiced)
    {
        pts[static_cast<size_t>(w)] = { inMidi, tgtMidi, voiced };
        w = (w + 1) % N; count = std::min(count + 1, N);
        if (voiced)
        {
            const float low = tgtMidi - 9.0f;
            viewLow = init ? viewLow + 0.03f * (low - viewLow) : low;
            init = true;
        }
    }
    void clear() { count = 0; w = 0; }
    void setScale(int rootMidi_, const std::array<float, 7>& d, float curTarget) { rootMidi = rootMidi_; degs = d; curTargetMidi = curTarget; }
    void setInfo(const juce::String& s) { info = s; }

    std::function<void(int)> onPickMidi;
    bool pencil = false;

    juce::Rectangle<float> plotArea() const
    {
        return getLocalBounds().toFloat().withTrimmedLeft(46.0f).withTrimmedTop(8.0f).withTrimmedBottom(8.0f).withTrimmedRight(8.0f);
    }

    void paint(juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();
        g.setColour(juce::Colour(0xff09132a)); g.fillRoundedRectangle(r, 10.0f);
        g.setColour(SC::goldDim.withAlpha(0.6f)); g.drawRoundedRectangle(r.reduced(0.5f), 10.0f, 1.2f);

        const auto plot = plotArea();
        const float span = 18.0f;
        auto yOf = [&](float m) { return plot.getBottom() - (m - viewLow) / span * plot.getHeight(); };

        g.saveState();
        g.reduceClipRegion(plot.toNearestInt());
        for (int x = static_cast<int>(plot.getX()); x < plot.getRight(); x += 46)
        { g.setColour(juce::Colour(0xff142240)); g.drawVerticalLine(x, plot.getY(), plot.getBottom()); }
        g.restoreState();

        for (int m = static_cast<int>(std::floor(viewLow)); m <= static_cast<int>(std::ceil(viewLow + span)); ++m)
        {
            const float y = yOf(static_cast<float>(m));
            if (y < plot.getY() || y > plot.getBottom()) continue;
            const auto name = juce::MidiMessage::getMidiNoteName(m, true, true, 4);
            const bool nat = !name.containsChar('#');
            g.setColour(juce::Colour(0xff1d2c4d).withAlpha(nat ? 0.9f : 0.35f));
            g.drawHorizontalLine(static_cast<int>(y), plot.getX(), plot.getRight());
            if (nat) { g.setColour(SC::dim); g.setFont(13.0f);
                       g.drawText(name, juce::Rectangle<float>(r.getX() + 4.0f, y - 8.0f, 36.0f, 16.0f), juce::Justification::centredRight, false); }
        }

        g.saveState();
        g.reduceClipRegion(plot.toNearestInt());

        // lad dərəcələri
        for (int k = -2; k <= 3; ++k)
            for (float d : degs)
            {
                const float m = static_cast<float>(rootMidi) + d / 100.0f + 12.0f * static_cast<float>(k);
                const float y = yOf(m);
                if (y < plot.getY() - 2 || y > plot.getBottom() + 2) continue;
                const bool cur = curTargetMidi > 0 && std::fabs(m - curTargetMidi) < 0.35f;
                g.setColour(SC::gold.withAlpha(cur ? 0.55f : 0.2f));
                g.drawLine(plot.getX(), y, plot.getRight(), y, cur ? 2.0f : 1.0f);
            }

        // giriş (kölgə zolağı) və düzəldilmiş əyri
        juce::Path pIn, pTg;
        bool prev = false;
        float lastX = plot.getRight();
        for (int i = 0; i < count; ++i)
        {
            const auto& q = pts[static_cast<size_t>((w - count + i + N) % N)];
            const float x = plot.getX() + plot.getWidth() * static_cast<float>(i + N - count) / static_cast<float>(N - 1);
            if (!q.voiced || q.tgt <= 0.0f) { prev = false; continue; }
            if (prev) { pIn.lineTo(x, yOf(q.in)); pTg.lineTo(x, yOf(q.tgt)); }
            else      { pIn.startNewSubPath(x, yOf(q.in)); pTg.startNewSubPath(x, yOf(q.tgt)); }
            prev = true; lastX = x;
        }
        g.setColour(SC::gold.withAlpha(0.16f));
        g.strokePath(pIn, juce::PathStrokeType(16.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour(SC::gold.withAlpha(0.22f));
        g.strokePath(pTg, juce::PathStrokeType(7.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour(SC::goldHi);
        g.strokePath(pTg, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        for (int i = 0; i < count; i += 9)
        {
            const auto& q = pts[static_cast<size_t>((w - count + i + N) % N)];
            if (!q.voiced || q.tgt <= 0.0f) continue;
            const float x = plot.getX() + plot.getWidth() * static_cast<float>(i + N - count) / static_cast<float>(N - 1);
            const float y = yOf(q.tgt);
            g.setColour(SC::gold); g.fillEllipse(x - 3.5f, y - 3.5f, 7.0f, 7.0f);
            g.setColour(juce::Colour(0xff09132a)); g.fillEllipse(x - 1.5f, y - 1.5f, 3.0f, 3.0f);
        }
        g.setColour(SC::text.withAlpha(0.5f));
        g.drawVerticalLine(static_cast<int>(lastX), plot.getY(), plot.getBottom());
        g.restoreState();

        g.setColour(SC::dim); g.setFont(13.0f);
        g.drawText(info, getLocalBounds().removeFromTop(30).withTrimmedRight(14), juce::Justification::centredRight, false);
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        if (!pencil || !onPickMidi) return;
        const auto plot = plotArea();
        const float m = viewLow + (plot.getBottom() - static_cast<float>(e.y)) / plot.getHeight() * 18.0f;
        onPickMidi(static_cast<int>(std::lround(m)));
    }

private:
    std::array<Pt, N> pts {};
    int w = 0, count = 0, rootMidi = 62;
    float viewLow = 60.0f, curTargetMidi = -1.0f;
    bool init = false;
    std::array<float, 7> degs { 0, 155, 300, 500, 700, 855, 1000 };
    juce::String info;
};

//==============================================================================  əsas interfeys (1280x714 dizayn koordinatları)
class SPitchUI : public juce::Component, private juce::Timer
{
public:
    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    using CA = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;

    explicit SPitchUI(SPitchProcessor& p) : proc(p), apvts(p.apvts)
    {
        setLookAndFeel(&laf);
        setSize(1280, 714);

        // --- yuxarı sətir
        setupCombo(ladC, "lad", caLad);       setupCombo(modC, "mod", caMod);
        setupCombo(mugamC, "mugam", caMugam); setupCombo(takrarC, "takrar", caTakrar);
        setupCombo(keyC, "key", caKey);       setupCombo(tuningC, "tuning", caTuning);

        addAndMakeVisible(presetC);
        presetC.onChange = [this] { if (!updatingPreset) applyPresetId(presetC.getSelectedId()); };
        refreshPresetList();
        presetC.setSelectedId(1, juce::dontSendNotification);

        for (auto* b : { &prevB, &nextB, &saveB, &undoB, &redoB, &penB, &rectB, &magB, &waveB, &gearB, &minB, &closeB })
            addAndMakeVisible(*b);
        prevB.onClick = [this] { stepPreset(-1); };
        nextB.onClick = [this] { stepPreset(1); };
        saveB.onClick = [this] { savePreset(); };
        undoB.onClick = [this] { proc.undoManager.undo(); };
        redoB.onClick = [this] { proc.undoManager.redo(); };
        penB.setClickingTogglesState(true);
        penB.onClick = [this] { graph.pencil = penB.getToggleState(); };
        rectB.onClick = [this] { graph.clear(); };
        magB.setClickingTogglesState(true);  baMag = std::make_unique<BA>(apvts, "aiPitch", magB);
        waveB.setClickingTogglesState(true); baWave = std::make_unique<BA>(apvts, "natural", waveB);
        gearB.onClick = [this] { showSettingsMenu(); };
        minB.onClick = [this] { if (auto* peer = getPeer()) peer->setMinimised(true); };
        closeB.onClick = [this] { if (auto* app = juce::JUCEApplicationBase::getInstance()) app->systemRequestedQuit(); };

        // --- naviqasiya
        const char* names[] = { "Pitch", "Correction", "Harmony", "Vibrato", "Tuning", "Formant", "EQ", "Effects", "Presets" };
        for (int i = 0; i < 9; ++i)
        {
            navB[static_cast<size_t>(i)] = std::make_unique<NavButton>(names[i], i);
            addAndMakeVisible(*navB[static_cast<size_t>(i)]);
            navB[static_cast<size_t>(i)]->onClick = [this, i] { selectNav(i); };
        }
        navB[0]->selected = true;

        // --- qrafik
        addAndMakeVisible(graph);
        graph.onPickMidi = [this](int midi) { SPitchParams::setReal(apvts, "key", static_cast<float>(((midi % 12) + 12) % 12)); };

        // --- Correction paneli
        knob(strengthS, "strength", "pct", saStrength);
        knob(tolS, "tolerance", "ct", saTol);
        linear(speedS, "speed", saSpeed);
        linear(transS, "transition", saTrans);
        addAndMakeVisible(naturalT); baNatural = std::make_unique<BA>(apvts, "natural", naturalT);
        setupCombo(vocalC, "vocalMode", caVocal);

        // --- alt panellər
        knob(harmS, "harmonyMix", "pct", saHarm);   setupCombo(voicesC, "voices", caVoices);
        knob(vibS, "vibDepth", "pct", saVib);       setupCombo(vibRateC, "vibRate", caVibRate);
        knob(formS, "formantAmt", "pct", saForm);   setupCombo(formShiftC, "formantShift", caFormShift);
        knob(revS, "reverbMix", "pct", saRev);      setupCombo(revTypeC, "reverbType", caRevType);
        knob(delS, "delayMix", "pct", saDel);       setupCombo(delDivC, "delayDiv", caDelDiv);
        knob(gainS, "gainDb", "knob", saGain);

        // --- əlavə səhifə (Tuning / EQ / Effects)
        knob(warmS, "warmth", "pct", saWarm); knob(airS, "air", "pct", saAir);
        knob(widthS, "width", "pct", saWidth); knob(driftS, "drift", "pct", saDrift);
        for (auto* s : { &warmS, &airS, &widthS, &driftS }) s->setVisible(false);

        for (auto* s : { &speedS, &transS, &tolS, &strengthS, &gainS })
            s->onValueChange = [this] { repaint(panelRepaintArea); };

        // --- alt panel: AI + güc
        addAndMakeVisible(powerB); powerB.setClickingTogglesState(true); baPower = std::make_unique<BA>(apvts, "bypass", powerB);
        powerB.painter = [](juce::Graphics& g, bool bypassed, juce::Rectangle<int> b)
        {
            auto r = b.toFloat().reduced(3.0f); const auto c = r.getCentre();
            g.setColour(juce::Colour(0xff0e1a33)); g.fillEllipse(r);
            g.setColour(bypassed ? SC::dim : SC::gold); g.drawEllipse(r, 2.0f);
            juce::Path p; p.addCentredArc(c.x, c.y, r.getWidth() * 0.27f, r.getWidth() * 0.27f, 0.0f, 0.6f, 5.68f, true);
            g.strokePath(p, juce::PathStrokeType(2.4f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
            g.drawLine(c.x, c.y - r.getWidth() * 0.32f, c.x, c.y - 1.0f, 2.4f);
        };

        addAndMakeVisible(analyzerB); analyzerB.setClickingTogglesState(true); baAnalyzer = std::make_unique<BA>(apvts, "aiAnalyzer", analyzerB);
        analyzerB.painter = [this](juce::Graphics& g, bool on, juce::Rectangle<int> b)
        {
            g.setColour(on ? SC::teal : SC::dim); g.setFont(juce::Font(14.0f));
            g.drawText("Milli AI Analiz", b.withHeight(22).withTrimmedLeft(10), juce::Justification::centredLeft, false);
            const float lvl = on ? std::clamp(proc.engine.getTelemetry().rmsLevel * 1.6f, 0.05f, 1.0f) : 0.05f;
            for (int i = 0; i < 16; ++i)
            {
                const float hgt = 4.0f + 20.0f * lvl * (0.35f + 0.65f * std::fabs(std::sin(phase + static_cast<float>(i) * 0.75f)));
                g.setColour((on ? SC::teal : SC::dim).withAlpha(0.35f + 0.65f * static_cast<float>(i) / 16.0f));
                g.fillRoundedRectangle(14.0f + static_cast<float>(i) * 8.5f, static_cast<float>(b.getHeight()) - 6.0f - hgt, 4.5f, hgt, 2.0f);
            }
        };
        addAndMakeVisible(restoreB); restoreB.setClickingTogglesState(true); baRestore = std::make_unique<BA>(apvts, "aiPitch", restoreB);
        restoreB.painter = [](juce::Graphics& g, bool on, juce::Rectangle<int> b)
        {
            g.setColour(on ? SC::text : SC::dim); g.setFont(juce::Font(14.0f));
            g.drawText("AI Vocal Restore", b.withHeight(26), juce::Justification::centred, false);
            g.setColour(on ? SC::teal : SC::dim); g.setFont(juce::Font(15.0f, juce::Font::bold));
            g.drawText("AI ENHANCE", b.withTrimmedTop(24), juce::Justification::centred, false);
        };

        addAndMakeVisible(presetsB);
        presetsB.painter = [](juce::Graphics& g, bool, juce::Rectangle<int> b)
        {
            auto r = b.toFloat().reduced(1.0f);
            g.setColour(juce::Colour(0xff0a1428)); g.fillRoundedRectangle(r, 12.0f);
            g.setColour(SC::goldDim); g.drawRoundedRectangle(r, 12.0f, 1.2f);
            g.setColour(SC::gold); drawNavIcon(g, 8, { 14.0f, 10.0f, 24.0f, r.getHeight() - 20.0f });
            g.setColour(SC::text); g.setFont(juce::Font(15.0f));
            g.drawText("Azerbaijan Presets", r.withTrimmedLeft(44.0f), juce::Justification::centredLeft, false);
        };
        presetsB.onClick = [this] { showPresetMenu(&presetsB); };

        startTimerHz(30);
    }

    ~SPitchUI() override { setLookAndFeel(nullptr); }

    //----------------------------------------------------------------------------- rəsm
    void paint(juce::Graphics& g) override
    {
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff0a1a38), 0, 0, juce::Colour(0xff04091a), 0, 714, false));
        g.fillAll();
        paintHeader(g);

        g.setColour(SC::goldDim.withAlpha(0.7f));
        g.drawRoundedRectangle(6.0f, 122.0f, 1268.0f, 530.0f, 16.0f, 1.4f);

        // başlıq sətirinin etiketləri
        txt(g, "Preset", { 45, 134, 100, 22 }, 15, SC::text, juce::Justification::centredLeft);
        g.setColour(SC::gold); g.fillPath(starPath(33.0f, 145.0f, 6.0f));
        txt(g, "Scale / Lad", { 452, 134, 130, 22 }, 15, SC::text, juce::Justification::centredLeft);
        rosette(g, 424.0f, 144.0f, 11.0f); rosette(g, 424.0f, 176.0f, 11.0f);
        txt(g, juce::String::fromUTF8("Muğam Ladanları"), { 780, 134, 160, 22 }, 15, SC::text, juce::Justification::centredLeft);
        txt(g, juce::String::fromUTF8("♪"), { 750, 132, 24, 24 }, 20, SC::gold);
        txt(g, "Key", { 1100, 134, 50, 22 }, 15, SC::text, juce::Justification::centredLeft);
        txt(g, juce::String::fromUTF8("♪"), { 1076, 132, 24, 24 }, 18, SC::gold);
        txt(g, "Tuning", { 1206, 134, 70, 22 }, 15, SC::text, juce::Justification::centredLeft);
        for (int i = 0; i < 4; ++i) { g.setColour(SC::gold); g.fillRect(1180 + i * 4, 140 + (i % 2) * 3, 2, 12 - (i % 2) * 6); }

        // qrafik çərçivəsi (əlavə səhifə üçün)
        g.setColour(juce::Colour(0xff09132a)); g.fillRoundedRectangle(183.0f, 208.0f, 825.0f, 292.0f, 10.0f);
        g.setColour(SC::goldDim.withAlpha(0.6f)); g.drawRoundedRectangle(183.5f, 208.5f, 824.0f, 291.0f, 10.0f, 1.2f);
        if (showExtra)
        {
            txt(g, "Tuning  ·  EQ  ·  Effects", { 200, 220, 500, 26 }, 18, SC::goldHi, juce::Justification::centredLeft);
            const char* ln[] = { "Warmth", "Air (4 kHz)", "Stereo Width", "Pitch Drift" };
            for (int i = 0; i < 4; ++i)
                txt(g, ln[i], { 183 + 825 * i / 4 + 20, 420, 185, 22 }, 16, SC::text);
        }

        // Correction paneli
        panelBox(g, { 1020, 208, 245, 292 });
        txt(g, "Correction", { 1034, 218, 150, 24 }, 17, SC::text, juce::Justification::centredLeft);
        txt(g, "Strength", { 1048, 338, 90, 20 }, 14, SC::text);
        txt(g, "Tolerance", { 1160, 246, 90, 18 }, 14, SC::text);
        txt(g, juce::String::fromUTF8("± ") + juce::String(juce::roundToInt(static_cast<float>(tolS.getValue()))) + " ct",
            { 1160, 330, 90, 18 }, 14, SC::text);
        txt(g, "Natural Mode", { 1034, 369, 120, 24 }, 14, SC::text, juce::Justification::centredLeft);
        txt(g, "Speed", { 1034, 398, 60, 20 }, 14, SC::text, juce::Justification::centredLeft);
        txt(g, juce::String(juce::roundToInt(static_cast<float>(speedS.getValue()))) + " ms", { 1202, 398, 56, 20 }, 14, SC::text, juce::Justification::centredRight);
        txt(g, "Transition", { 1034, 424, 70, 20 }, 14, SC::text, juce::Justification::centredLeft);
        txt(g, juce::String(juce::roundToInt(static_cast<float>(transS.getValue()))) + " ms", { 1202, 424, 56, 20 }, 14, SC::text, juce::Justification::centredRight);
        g.setColour(SC::goldDim.withAlpha(0.6f)); g.drawHorizontalLine(458, 1040.0f, 1250.0f);
        rosette(g, 1135.0f, 458.0f, 6.0f);
        txt(g, juce::String::fromUTF8("♬"), { 1030, 470, 22, 24 }, 18, SC::gold);
        txt(g, "Vocal Mode", { 1052, 474, 70, 22 }, 12, SC::text, juce::Justification::centredLeft);

        // alt panellər
        const char* titles[] = { "Harmony", "Vibrato", "Formant", "Reverb", "Delay", "Master Gain" };
        for (int i = 0; i < 6; ++i)
        {
            const auto& pr = bottomPanels[static_cast<size_t>(i)];
            panelBox(g, pr);
            if (i == highlightIdx && highlightAlpha > 0.0f)
            { g.setColour(SC::goldHi.withAlpha(highlightAlpha)); g.drawRoundedRectangle(pr.toFloat(), 14.0f, 2.5f); }
            txt(g, titles[i], { pr.getX() + 38, pr.getY() + 6, pr.getWidth() - 40, 26 }, 16, SC::text, juce::Justification::centredLeft);
            g.setColour(SC::gold); drawNavIcon(g, i == 0 ? 2 : (i == 1 ? 3 : (i == 2 ? 5 : (i == 3 ? 7 : (i == 4 ? 1 : 4)))), { pr.getX() + 10.0f, pr.getY() + 8.0f, 22.0f, 20.0f });
        }
        txt(g, "Mix", { 204, 620, 74, 20 }, 14, SC::dim);   // Harmony
        txt(g, "Voices", { 296, 552, 60, 18 }, 13, SC::dim);
        txt(g, "Depth", { 394, 620, 70, 20 }, 14, SC::dim);
        txt(g, "Rate", { 480, 584, 60, 16 }, 13, SC::dim, juce::Justification::centredLeft);
        txt(g, "Amount", { 628, 620, 70, 20 }, 14, SC::dim);
        txt(g, "Shift", { 706, 560, 56, 16 }, 13, SC::dim);
        txt(g, "Mix", { 796, 620, 70, 20 }, 14, SC::dim);
        txt(g, "Type", { 872, 570, 52, 16 }, 13, SC::dim);
        txt(g, "Mix", { 955, 620, 70, 20 }, 14, SC::dim);
        txt(g, "Time", { 1030, 570, 50, 16 }, 13, SC::dim);
        txt(g, "Master Gain", { 1100, 622, 125, 20 }, 14, SC::text);
        txt(g, juce::String(proc.apvts.getRawParameterValue("gainDb")->load(), 1) + " dB", { 1100, 524, 125, 18 }, 12, SC::dim);
        paintVibWave(g, { 480.0f, 542.0f, 110.0f, 36.0f });

        paintMeter(g);

        // alt zolaq
        g.setColour(juce::Colour(0xff08122a)); g.fillRoundedRectangle(8.0f, 655.0f, 1264.0f, 52.0f, 14.0f);
        g.setColour(SC::goldDim.withAlpha(0.6f)); g.drawRoundedRectangle(8.0f, 655.0f, 1264.0f, 52.0f, 14.0f, 1.2f);
        const bool byp = apvts.getRawParameterValue("bypass")->load() > 0.5f;
        txt(g, "SPitch Pro", { 100, 662, 90, 22 }, 15, SC::text, juce::Justification::centredLeft);
        txt(g, juce::String::fromUTF8("•  ") + juce::String(byp ? "Bypassed" : "Active Engine"), { 186, 662, 150, 22 }, 15, byp ? SC::gold : SC::green, juce::Justification::centredLeft);
        txt(g, juce::String(proc.latencyMs.load(), 1) + "ms Latency", { 100, 684, 150, 18 }, 13, SC::dim, juce::Justification::centredLeft);
        txt(g, "CPU: " + juce::String(juce::roundToInt(proc.cpuLoad.load() * 100.0f)) + "%", { 260, 684, 100, 18 }, 13, SC::dim, juce::Justification::centredLeft);
        g.setColour(SC::teal.withAlpha(0.5f)); g.drawRoundedRectangle(490.0f, 660.0f, 278.0f, 48.0f, 10.0f, 1.2f);
        g.setFont(juce::Font("Georgia", 19.0f, juce::Font::italic));
        g.setColour(SC::goldHi.withAlpha(0.9f));
        g.drawText(juce::String::fromUTF8("Muğam ruhu,\nmüasir texnologiya..."), { 985, 658, 280, 48 }, juce::Justification::centredRight, true);

        // sol aşağı loqo
        medallion(g, 90.0f, 552.0f);
        txt(g, "SPitch", { 20, 590, 140, 34 }, 28, SC::gold);
        txt(g, "by Azerbaijani Soul", { 20, 622, 140, 20 }, 13, SC::text);
    }

    void resized() override
    {
        presetC.setBounds(27, 158, 242, 34);
        prevB.setBounds(276, 158, 32, 34); nextB.setBounds(310, 158, 32, 34); saveB.setBounds(344, 158, 32, 34);
        ladC.setBounds(447, 158, 118, 34);   modC.setBounds(586, 158, 127, 34);
        mugamC.setBounds(747, 158, 142, 34); takrarC.setBounds(903, 158, 106, 34);
        keyC.setBounds(1054, 158, 88, 34);   tuningC.setBounds(1164, 158, 88, 34);

        gearB.setBounds(1112, 16, 38, 38); minB.setBounds(1176, 16, 32, 32); closeB.setBounds(1226, 16, 32, 32);

        for (int i = 0; i < 9; ++i) navB[static_cast<size_t>(i)]->setBounds(14, 221 + static_cast<int>(32.8f * static_cast<float>(i)), 156, 31);

        graph.setBounds(183, 208, 825, 292);
        undoB.setBounds(232, 212, 34, 32); redoB.setBounds(268, 212, 34, 32); penB.setBounds(304, 212, 34, 32);
        rectB.setBounds(340, 212, 34, 32); magB.setBounds(376, 212, 34, 32); waveB.setBounds(412, 212, 34, 32);

        strengthS.setBounds(1048, 250, 90, 90); tolS.setBounds(1176, 268, 58, 58);
        naturalT.setBounds(1206, 369, 44, 24);
        speedS.setBounds(1094, 398, 108, 20); transS.setBounds(1094, 424, 108, 20);
        vocalC.setBounds(1118, 472, 138, 28);

        harmS.setBounds(204, 546, 74, 74);  voicesC.setBounds(296, 574, 54, 28);
        vibS.setBounds(394, 546, 74, 74);   vibRateC.setBounds(480, 602, 100, 28);
        formS.setBounds(628, 546, 74, 74);  formShiftC.setBounds(706, 576, 56, 28);
        revS.setBounds(796, 546, 74, 74);   revTypeC.setBounds(872, 588, 52, 28);
        delS.setBounds(955, 546, 74, 74);   delDivC.setBounds(1030, 588, 50, 28);
        gainS.setBounds(1112, 544, 80, 80);

        for (int i = 0; i < 4; ++i)
        {
            auto* s = std::array<juce::Slider*, 4> { &warmS, &airS, &widthS, &driftS }[static_cast<size_t>(i)];
            s->setBounds(183 + 825 * i / 4 + 40, 270, 140, 140);
        }

        powerB.setBounds(18, 659, 44, 44);
        analyzerB.setBounds(490, 660, 170, 48); restoreB.setBounds(662, 660, 106, 48);
        presetsB.setBounds(782, 666, 186, 38);
    }

private:
    //----------------------------------------------------------------------------- yardımçılar
    static void txt(juce::Graphics& g, const juce::String& t, juce::Rectangle<int> r, float size, juce::Colour c,
                    juce::Justification j = juce::Justification::centred)
    { g.setColour(c); g.setFont(juce::Font(size)); g.drawText(t, r, j, false); }

    static juce::Path starPath(float cx, float cy, float rad)
    { juce::Path p; p.addStar({ cx, cy }, 5, rad * 0.45f, rad, 0.0f); return p; }

    static void rosette(juce::Graphics& g, float cx, float cy, float r)
    {
        g.setColour(SC::gold.withAlpha(0.85f));
        for (int i = 0; i < 6; ++i)
        {
            const float a = static_cast<float>(i) * juce::MathConstants<float>::pi / 3.0f;
            g.drawEllipse(cx + std::cos(a) * r * 0.45f - r * 0.4f, cy + std::sin(a) * r * 0.45f - r * 0.4f, r * 0.8f, r * 0.8f, 1.0f);
        }
    }

    static void panelBox(juce::Graphics& g, juce::Rectangle<int> r)
    {
        g.setColour(SC::panel); g.fillRoundedRectangle(r.toFloat(), 14.0f);
        g.setColour(SC::goldDim.withAlpha(0.65f)); g.drawRoundedRectangle(r.toFloat().reduced(0.5f), 14.0f, 1.2f);
    }

    void medallion(juce::Graphics& g, float cx, float cy)
    {
        g.setColour(SC::gold.withAlpha(0.9f));
        for (int i = 0; i < 8; ++i)
        {
            juce::Path p; p.addEllipse(cx - 7.0f, cy - 38.0f, 14.0f, 30.0f);
            p.applyTransform(juce::AffineTransform::rotation(static_cast<float>(i) * 0.785f, cx, cy));
            g.strokePath(p, juce::PathStrokeType(1.4f));
        }
        g.drawEllipse(cx - 14, cy - 14, 28, 28, 1.6f);
        g.drawEllipse(cx - 44, cy - 44, 88, 88, 1.0f);
    }

    void paintVibWave(juce::Graphics& g, juce::Rectangle<float> r)
    {
        juce::Path p;
        const float depth = static_cast<float>(vibS.getValue());
        for (int i = 0; i <= 60; ++i)
        {
            const float x = r.getX() + r.getWidth() * static_cast<float>(i) / 60.0f;
            const float y = r.getCentreY() + std::sin(static_cast<float>(i) * 0.9f + phase) * r.getHeight() * 0.4f * (0.2f + depth)
                          * (0.5f + 0.5f * std::sin(static_cast<float>(i) * 0.15f));
            if (i == 0) p.startNewSubPath(x, y); else p.lineTo(x, y);
        }
        g.setColour(SC::gold.withAlpha(0.85f)); g.strokePath(p, juce::PathStrokeType(1.4f));
    }

    void paintMeter(juce::Graphics& g)
    {
        const auto m = meterRect.toFloat();
        g.setColour(juce::Colour(0xff0a1226)); g.fillRoundedRectangle(m, 4.0f);
        const float db = 20.0f * std::log10(std::max(proc.outLevel.load(), 1.0e-4f));
        const float f = juce::jmap(std::clamp(db, -60.0f, 0.0f), -60.0f, 0.0f, 0.0f, 1.0f);
        const int segs = 24;
        for (int i = 0; i < segs; ++i)
        {
            const float y = m.getBottom() - 3.0f - static_cast<float>(i + 1) * (m.getHeight() - 6.0f) / static_cast<float>(segs);
            const bool lit = static_cast<float>(i) / static_cast<float>(segs) < f;
            g.setColour((i > 20 ? juce::Colour(0xffe85a4a) : SC::teal).withAlpha(lit ? 0.95f : 0.12f));
            g.fillRect(m.getX() + 2.0f, y, m.getWidth() - 4.0f, (m.getHeight() - 6.0f) / static_cast<float>(segs) - 1.5f);
        }
        txt(g, "dB", { meterRect.getX() - 6, meterRect.getBottom() + 2, 28, 16 }, 12, SC::dim);
    }

    void paintHeader(juce::Graphics& g)
    {
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff0c1e42), 0, 0, juce::Colour(0xff071126), 1280, 0, false));
        g.fillRect(0, 0, 1280, 118);

        // kafel / pətək naxışı
        g.setColour(SC::teal.withAlpha(0.13f));
        for (int j = 0; j < 6; ++j)
            for (int i = 0; i < 14; ++i)
            {
                const float x = 330.0f + static_cast<float>(i) * 18.0f + (j % 2) * 9.0f, y = 8.0f + static_cast<float>(j) * 18.0f;
                juce::Path d; d.addQuadrilateral(x, y - 9, x + 9, y, x, y + 9, x - 9, y); g.strokePath(d, juce::PathStrokeType(0.8f));
            }

        // buta (palıdı damla) naxışları
        for (int k = 0; k < 2; ++k)
        {
            juce::Path b;
            b.startNewSubPath(0, 0);
            b.cubicTo(60, -10, 70, 70, 20, 100);
            b.cubicTo(-30, 125, -55, 60, -15, 30);
            b.cubicTo(5, 12, 10, 8, 0, 0);
            b.applyTransform(juce::AffineTransform::scale(0.8f).rotated(k == 0 ? 0.4f : -0.3f).translated(k == 0 ? 640.0f : 820.0f, 8.0f));
            g.setColour(SC::gold.withAlpha(0.18f)); g.fillPath(b);
            g.setColour(SC::gold.withAlpha(0.8f)); g.strokePath(b, juce::PathStrokeType(1.6f));
        }

        g.setColour(SC::gold); g.fillEllipse(24.0f, 22.0f, 74.0f, 74.0f);
        g.setColour(SC::bg0); g.fillEllipse(28.0f, 26.0f, 66.0f, 66.0f);
        txt(g, juce::String::fromUTF8("♫"), { 28, 26, 66, 66 }, 44, SC::gold);

        g.setFont(juce::Font("Georgia", 56.0f, juce::Font::bold));
        g.setColour(SC::goldHi);
        g.drawText("SPitch", 128, 10, 250, 66, juce::Justification::centredLeft, false);
        txt(g, "Vocal · Pitch · Harmony · Culture", { 130, 76, 300, 20 }, 13, SC::text, juce::Justification::centredLeft);

        g.setFont(juce::Font("Brush Script MT", 62.0f, juce::Font::italic));
        g.setColour(SC::goldHi);
        g.drawText(SC::kArtistName, 640, 14, 340, 80, juce::Justification::centred, false);

        // Azərbaycan bayrağı
        g.setColour(juce::Colour(0xffb0b8c8)); g.fillRect(1004, 26, 3, 86);
        g.setColour(juce::Colour(0xff00b5e2)); g.fillRect(1007, 30, 112, 20);
        g.setColour(juce::Colour(0xffef3340)); g.fillRect(1007, 50, 112, 20);
        g.setColour(juce::Colour(0xff509e2f)); g.fillRect(1007, 70, 112, 20);
        g.setColour(juce::Colours::white); g.fillEllipse(1049.0f, 53.0f, 14.0f, 14.0f);
        g.setColour(juce::Colour(0xffef3340)); g.fillEllipse(1053.0f, 54.0f, 12.0f, 12.0f);
        g.setColour(juce::Colours::white); g.fillPath(starPath(1073.0f, 60.0f, 4.0f));

        g.setColour(SC::goldDim); g.drawRoundedRectangle(1150.0f, 62.0f, 108.0f, 30.0f, 8.0f, 1.2f);
        txt(g, "Professional", { 1150, 62, 108, 30 }, 15, SC::text);
        g.setColour(SC::goldDim.withAlpha(0.8f)); g.drawHorizontalLine(118, 0.0f, 1280.0f);
    }

    //----------------------------------------------------------------------------- idarəetmə qurğuları
    void setupCombo(juce::ComboBox& cb, const char* id, std::unique_ptr<CA>& att)
    {
        if (auto* cp = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(id))) cb.addItemList(cp->choices, 1);
        addAndMakeVisible(cb);
        att = std::make_unique<CA>(apvts, id, cb);
    }

    void knob(juce::Slider& s, const char* id, const char* kind, std::unique_ptr<SA>& att)
    {
        s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        s.setRotaryParameters(juce::MathConstants<float>::pi * 1.2f, juce::MathConstants<float>::pi * 2.8f, true);
        s.getProperties().set("kind", kind);
        if (auto* p = apvts.getParameter(id)) s.setDoubleClickReturnValue(true, p->convertFrom0to1(p->getDefaultValue()));
        addAndMakeVisible(s);
        att = std::make_unique<SA>(apvts, id, s);
    }

    void linear(juce::Slider& s, const char* id, std::unique_ptr<SA>& att)
    {
        s.setSliderStyle(juce::Slider::LinearHorizontal);
        s.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        if (auto* p = apvts.getParameter(id)) s.setDoubleClickReturnValue(true, p->convertFrom0to1(p->getDefaultValue()));
        addAndMakeVisible(s);
        att = std::make_unique<SA>(apvts, id, s);
    }

    //----------------------------------------------------------------------------- naviqasiya
    void selectNav(int i)
    {
        if (i == 8) { showPresetMenu(navB[8].get()); return; }
        for (int k = 0; k < 9; ++k) navB[static_cast<size_t>(k)]->selected = (k == i);
        for (auto& b : navB) b->repaint();

        showExtra = (i == 4 || i == 6 || i == 7);
        graph.setVisible(!showExtra);
        for (auto* s : { &warmS, &airS, &widthS, &driftS }) s->setVisible(showExtra);
        if (i == 2) flash(0); else if (i == 3) flash(1); else if (i == 5) flash(2);
        repaint();
    }

    void flash(int panelIdx) { highlightIdx = panelIdx; highlightAlpha = 1.0f; }

    //----------------------------------------------------------------------------- presetlər
    void refreshPresetList()
    {
        updatingPreset = true;
        presetC.clear(juce::dontSendNotification);
        const auto& all = SPitchParams::factoryPresets();
        for (size_t i = 0; i < all.size(); ++i) presetC.addItem(SPitchParams::U8(all[i].name), static_cast<int>(i) + 1);
        userFiles.clear();
        presetC.addSeparator();
        for (auto& f : SPitchParams::userPresetDir().findChildFiles(juce::File::findFiles, false, "*.spreset"))
        { userFiles.add(f); presetC.addItem(f.getFileNameWithoutExtension() + "  (user)", 100 + userFiles.size() - 1); }
        updatingPreset = false;
    }

    void applyPresetId(int id)
    {
        if (id >= 100)
        {
            const auto f = userFiles[id - 100];
            if (auto xml = juce::XmlDocument::parse(f))
            { proc.undoManager.beginNewTransaction(); apvts.replaceState(juce::ValueTree::fromXml(*xml)); }
        }
        else SPitchParams::applyFactoryPreset(proc, apvts, proc.undoManager, id - 1);
    }

    void stepPreset(int dir)
    {
        const int n = presetC.getNumItems();
        if (n == 0) return;
        const int cur = presetC.getSelectedItemIndex();
        presetC.setSelectedItemIndex(((cur < 0 ? 0 : cur) + dir + n) % n, juce::sendNotification);
    }

    void showPresetMenu(juce::Component* target)
    {
        juce::PopupMenu m;
        const auto& all = SPitchParams::factoryPresets();
        for (size_t i = 0; i < all.size(); ++i) m.addItem(static_cast<int>(i) + 1, SPitchParams::U8(all[i].name));
        if (userFiles.size() > 0) m.addSeparator();
        for (int i = 0; i < userFiles.size(); ++i) m.addItem(100 + i, userFiles[i].getFileNameWithoutExtension());
        m.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(target),
                        [this](int id) { if (id > 0) presetC.setSelectedId(id, juce::sendNotification); });
    }

    void savePreset()
    {
        auto* aw = new juce::AlertWindow("Presetı saxla", "Preset adı:", juce::MessageBoxIconType::NoIcon);
        aw->addTextEditor("name", "Mənim presetim");
        aw->addButton("Saxla", 1, juce::KeyPress(juce::KeyPress::returnKey));
        aw->addButton("Ləğv et", 0, juce::KeyPress(juce::KeyPress::escapeKey));
        juce::Component::SafePointer<SPitchUI> safe(this);
        aw->enterModalState(true, juce::ModalCallbackFunction::create([safe, aw](int result)
        {
            if (result == 1 && safe != nullptr)
            {
                auto name = aw->getTextEditorContents("name").trim();
                if (name.isNotEmpty())
                {
                    auto file = SPitchParams::userPresetDir().getChildFile(juce::File::createLegalFileName(name) + ".spreset");
                    if (auto xml = safe->apvts.copyState().createXml()) xml->writeTo(file);
                    safe->refreshPresetList();
                    for (int i = 0; i < safe->userFiles.size(); ++i)
                        if (safe->userFiles[i] == file) safe->presetC.setSelectedId(100 + i, juce::dontSendNotification);
                }
            }
        }), true);
    }

    void showSettingsMenu()
    {
        juce::PopupMenu m;
        const bool autoRoot = apvts.getRawParameterValue("autoRoot")->load() > 0.5f;
        m.addItem(1, juce::String::fromUTF8("Avtomatik kök (tonika) tapılsın"), true, autoRoot);
        m.addItem(2, juce::String::fromUTF8("Analizatoru sıfırla"));
        m.addItem(3, juce::String::fromUTF8("Pitch qrafikini təmizlə"));
        m.showMenuAsync(juce::PopupMenu::Options().withTargetComponent(&gearB), [this](int id)
        {
            if (id == 1) { if (auto* p = apvts.getParameter("autoRoot")) p->setValueNotifyingHost(p->getValue() > 0.5f ? 0.0f : 1.0f); }
            else if (id == 2) proc.resetRequested.store(true);
            else if (id == 3) graph.clear();
        });
    }

    //----------------------------------------------------------------------------- taymer
    void timerCallback() override
    {
        const auto t = proc.engine.getTelemetry();
        const float ref = SPitchParams::kTuning[std::clamp(static_cast<int>(apvts.getRawParameterValue("tuning")->load() + 0.5f), 0, 9)];
        auto toMidi = [ref](float hz) { return 69.0f + 12.0f * std::log2(std::max(hz, 1.0f) / ref); };

        graph.setScale(t.detectedRootMidi, SPitchEngine::ladDegrees(t.recommendedLad), t.voiced && t.targetHz > 0 ? toMidi(t.targetHz) : -1.0f);
        graph.setInfo(SPitchParams::U8(SPitchEngine::ladToString(t.recommendedLad)) + "  ·  "
                      + juce::MidiMessage::getMidiNoteName(t.detectedRootMidi, true, true, 4) + "  ·  "
                      + juce::String(juce::roundToInt(ref)) + " Hz");
        graph.push(t.inputHz > 0 ? toMidi(t.inputHz) : 0.0f, t.targetHz > 0 ? toMidi(t.targetHz) : 0.0f, t.voiced);
        graph.repaint();

        phase += 0.18f;
        analyzerB.repaint();
        repaint(meterRect.expanded(8, 20));
        if (++tick % 8 == 0) repaint(0, 655, 480, 52);
        if (highlightAlpha > 0.0f) { highlightAlpha = std::max(0.0f, highlightAlpha - 0.02f); repaint(176, 505, 1100, 146); }
        repaint(480, 540, 120, 40);   // vibrato dalğası
    }

    //----------------------------------------------------------------------------- üzvlər
    SPitchProcessor& proc;
    juce::AudioProcessorValueTreeState& apvts;
    SPitchLAF laf;

    PitchGraph graph;
    juce::ComboBox presetC, ladC, modC, mugamC, takrarC, keyC, tuningC, vocalC, voicesC, vibRateC, formShiftC, revTypeC, delDivC;
    juce::Slider strengthS, tolS, speedS, transS, harmS, vibS, formS, revS, delS, gainS, warmS, airS, widthS, driftS;
    juce::ToggleButton naturalT;

    IconButton undoB { 0 }, redoB { 1 }, penB { 2 }, rectB { 3 }, magB { 4 }, waveB { 5 }, gearB { 6 }, minB { 7 }, closeB { 8 },
               prevB { 9 }, nextB { 10 }, saveB { 11 };
    std::array<std::unique_ptr<NavButton>, 9> navB;
    FlatToggle powerB, analyzerB, restoreB, presetsB;

    std::unique_ptr<CA> caLad, caMod, caMugam, caTakrar, caKey, caTuning, caVocal, caVoices, caVibRate, caFormShift, caRevType, caDelDiv;
    std::unique_ptr<SA> saStrength, saTol, saSpeed, saTrans, saHarm, saVib, saForm, saRev, saDel, saGain, saWarm, saAir, saWidth, saDrift;
    std::unique_ptr<BA> baNatural, baMag, baWave, baPower, baAnalyzer, baRestore;

    juce::Array<juce::File> userFiles;
    bool updatingPreset = false, showExtra = false;
    int highlightIdx = -1, tick = 0;
    float highlightAlpha = 0.0f, phase = 0.0f;

    const std::array<juce::Rectangle<int>, 6> bottomPanels {
        juce::Rectangle<int>(182, 510, 178, 136), juce::Rectangle<int>(372, 510, 224, 136), juce::Rectangle<int>(607, 510, 158, 136),
        juce::Rectangle<int>(777, 510, 151, 136), juce::Rectangle<int>(940, 510, 146, 136), juce::Rectangle<int>(1098, 510, 170, 136) };
    const juce::Rectangle<int> meterRect { 1238, 522, 16, 104 };
    const juce::Rectangle<int> panelRepaintArea { 1020, 208, 245, 292 };
};

//==============================================================================
class SPitchEditor : public juce::AudioProcessorEditor
{
public:
    explicit SPitchEditor(SPitchProcessor& p) : juce::AudioProcessorEditor(&p), ui(p)
    {
        addAndMakeVisible(ui);
        setResizable(true, true);
        setResizeLimits(640, 357, 2560, 1428);
        getConstrainer()->setFixedAspectRatio(1280.0 / 714.0);
        setSize(1280, 714);
    }

    void resized() override
    {
        ui.setBounds(0, 0, 1280, 714);
        ui.setTransform(juce::AffineTransform::scale(static_cast<float>(getWidth()) / 1280.0f));
    }

private:
    SPitchUI ui;
};
