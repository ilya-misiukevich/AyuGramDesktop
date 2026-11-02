#include "ayu/ui/boxes/app_icon_box.h"

#include "lang_auto.h"
#include "tray.h"
#include "ayu/ayu_settings.h"
#include "ayu/ui/ayu_logo.h"
#include "base/flat_map.h"
#include "base/unique_qptr.h"
#include "core/application.h"
#include "info/info_wrap_widget.h"
#include "info/profile/tabs/info_profile_tabs_strip.h"
#include "info/settings/info_settings_widget.h"
#include "lang/lang_keys.h"
#include "main/main_domain.h"
#include "settings/settings_common.h"
#include "settings/settings_common_session.h"
#include "styles/style_app_icon_box.h"
#include "styles/style_ayu_styles.h"
#include "styles/style_info.h"
#include "styles/style_layers.h"
#include "styles/style_menu_icons.h"
#include "styles/style_settings.h"
#include "ui/color_contrast.h"
#include "ui/effects/animation_value.h"
#include "ui/effects/animations.h"
#include "ui/painter.h"
#include "ui/text/text_utilities.h"
#include "ui/ui_utility.h"
#include "ui/vertical_list.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/labels.h"
#include "ui/widgets/scroll_area.h"
#include "ui/wrap/fade_wrap.h"
#include "ui/wrap/slide_wrap.h"
#include "ui/wrap/vertical_layout.h"
#include "window/main_window.h"
#include "window/window_session_controller.h"

#ifdef Q_OS_WIN
#include "ayu/utils/windows_utils.h"
#endif

#include <QSvgRenderer>
#include <QtWidgets/QApplication>
#include <QtWidgets/QScrollBar>

namespace Settings {
namespace {

using SectionCustomTopBarData = Info::Settings::SectionCustomTopBarData;

constexpr auto kCaptionLines = 2;
constexpr auto kMinColumns = 3;
constexpr auto kMaxColumns = 8;
constexpr auto kAccentSize = 24; // AppIcons.accent samples a 24x24 bitmap
constexpr auto kTintDuration = crl::time(280);
constexpr auto kScrollStepTime = crl::time(260);
constexpr auto kTextCollapsePart = 0.35;
constexpr auto kGlowRadius = 0.9;
constexpr auto kMinAcceptableContrast = 4.5; // as in premium_top_bar.cpp

struct IconInfo {
	QString id;
	QString name;
	QString nameRu;
	QString info;
	QString author;
};

struct IconGroup {
	QString title;
	std::vector<IconInfo> icons;
};

[[nodiscard]] QString DefaultIcon() {
	return u"monet_extera_tg_light"_q;
}

[[nodiscard]] const std::vector<IconGroup> &Groups() {
	static const auto result = std::vector<IconGroup>{
		{ u"Monet"_q, {
			{ u"monet_extera_tg_light"_q, u"Sky"_q },
			{ u"monet_extera_tg_dark"_q, u"Sky Dark"_q },
			{ u"monet_extera_light"_q, u"Monet"_q },
			{ u"monet_extera_dark"_q, u"Monet Dark"_q },
			{ u"monet_extera_android_light"_q, u"Mint"_q },
			{ u"monet_extera_android_dark"_q, u"Mint Dark"_q },
		} },
		{ u"AyuGram"_q, {
			{ u"default"_q, u"AyuGram"_q },
			{ u"alt"_q, u"AyuGram Alt"_q },
			{ u"discord"_q, u"Discord"_q },
			{ u"spotify"_q, u"Spotify"_q },
			{ u"extera"_q, u"exteraGram"_q },
			{ u"nothing"_q, u"Nothing"_q },
			{ u"bard"_q, u"Google Bard"_q },
			{ u"yaplus"_q, u"Yandex Plus"_q },
			{ u"win95"_q, u"Windows 95"_q },
			{ u"chibi"_q, u"AyuTyan"_q },
			{ u"tyan"_q, u"Chibi"_q },
			{ u"chibi2"_q, u"Chibi Alt"_q },
		} },
		{ u"exteraGram"_q, {
			{ u"beta"_q, u"Beta"_q },
			{ u"winter"_q, u"Winter"_q, u"Зима"_q, u"Winter is coming"_q },
			{ u"extera2"_q, u"exteraGram"_q },
			{ u"orbit"_q, u"Orbit"_q, u"Орбита"_q, u"To infinity and beyond"_q },
			{ u"editor"_q, u"Editor"_q, u"Редактор"_q, u"final_v2_FINAL.fig"_q },
			{ u"mole"_q, u"Mole"_q, {}, {}, u"whatwherewhywhen"_q },
			{ u"echpochmak"_q, u"Echpochmak"_q, {}, {}, u"whatwherewhywhen"_q },
			{ u"avatar"_q, u"Avatar"_q, {}, {}, u"cIick_cIack"_q },
			{ u"candy"_q, u"Candy"_q, {}, {}, u"impulz3"_q },
			{ u"guitar"_q, u"Guitar"_q, {}, {}, u"nekto_nkt"_q },
			{ u"sus"_q, u"Impostor"_q, u"Предатель"_q, {}, u"MilesBlooper"_q },
			{ u"frog"_q, u"Frog"_q, {}, {}, u"cIick_cIack"_q },
			{ u"hamster"_q, u"Hamster"_q, {}, {}, u"whatwherewhywhen"_q },
			{ u"sponge"_q, u"Sponge"_q, {}, {}, u"nneeuurr"_q },
			{ u"invincible"_q, u"Invincible"_q, u"Неуязвимый"_q, u"Think, Mark!"_q },
			{ u"flappy"_q, u"Flappy"_q, {}, {}, u"nneeuurr"_q },
			{ u"nyan_cat"_q, u"Nyan Cat"_q, {}, {}, u"n1ksly"_q },
			{ u"ice_age"_q, u"Ice Age"_q, u"Ледник"_q, u"Where’s my acorn?"_q },
			{ u"fish"_q, u"Fish"_q, {}, {}, u"whatwherewhywhen"_q },
			{ u"psychosis"_q, u"Psychosis"_q, {}, {}, u"cIick_cIack"_q },
			{ u"xp"_q, u"XP"_q, {}, {}, u"weizoqq"_q },
			{ u"nasa"_q, u"NASA"_q, {}, {}, u"cIick_cIack"_q },
			{ u"cyberpunk"_q, u"Cyberpunk"_q, u"Киберпанк"_q, u"Wake up, samurai"_q },
			{ u"space"_q, u"Space"_q, u"Космос"_q, u"The Final Frontier"_q },
			{ u"jellyfish"_q, u"Jellyfish"_q, {}, {}, u"whatwherewhywhen"_q },
			{ u"human"_q, u"Human"_q, {}, {}, u"cIick_cIack"_q },
			{ u"sunset"_q, u"Sunset"_q, u"Закат"_q, u"Here comes the sun"_q },
			{ u"amethyst"_q, u"Amethyst"_q, u"Аметист"_q, u"Perfectly balanced"_q },
			{ u"aurora"_q, u"Aurora"_q, u"Аврора"_q, u"Localized entirely within your taskbar"_q },
			{ u"osu"_q, u"osu!"_q, {}, {}, u"ataeshi"_q },
			{ u"google"_q, u"Google"_q, {}, u"Don’t be evil"_q },
			{ u"google26"_q, u"Google ’26"_q, {}, {}, u"Slucket"_q },
		} },
		{ u"Telegram"_q, {
			{ u"telegram"_q, u"Desktop"_q },
			{ u"tg_android"_q, u"Telegram"_q },
			{ u"telegram_x"_q, u"Telegram X"_q },
			{ u"tg_vintage"_q, u"Vintage"_q },
			{ u"tg_aqua"_q, u"Aqua"_q },
			{ u"tg_premium"_q, u"Premium"_q },
			{ u"tg_turbo"_q, u"Turbo"_q },
			{ u"tg_nox"_q, u"Nox"_q },
		} },
	};
	return result;
}

[[nodiscard]] const IconInfo *FindIcon(const QString &id) {
	for (const auto &group : Groups()) {
		for (const auto &icon : group.icons) {
			if (icon.id == id) {
				return &icon;
			}
		}
	}
	return nullptr;
}

[[nodiscard]] int GroupIndex(const QString &id) {
	const auto &groups = Groups();
	for (auto i = 0, count = int(groups.size()); i != count; ++i) {
		for (const auto &icon : groups[i].icons) {
			if (icon.id == id) {
				return i;
			}
		}
	}
	return 0;
}

[[nodiscard]] QString CurrentIcon() {
	const auto &id = AyuSettings::getInstance().appIcon();
	return FindIcon(id) ? id : DefaultIcon();
}

[[nodiscard]] QString IconName(const QString &id) {
	const auto icon = FindIcon(id);
	if (!icon) {
		return QString();
	}
	const auto russian = Lang::Id().startsWith(u"ru"_q);
	return (russian && !icon->nameRu.isEmpty()) ? icon->nameRu : icon->name;
}

[[nodiscard]] TextWithEntities IconSubtitle(const QString &id) {
	const auto icon = FindIcon(id);
	if (!icon) {
		return {};
	} else if (!icon->info.isEmpty()) {
		return { icon->info };
	} else if (!icon->author.isEmpty()) {
		return Ui::Text::Link(
			u"@"_q + icon->author,
			u"https://t.me/"_q + icon->author);
	}
	return {};
}

void DrawIcon(QPainter &p, const QString &id, QRect rect, int pixels) {
	const auto png = u":/gui/art/ayu/%1/app.png"_q.arg(id);
	if (QFile::exists(png)) {
		p.drawImage(rect, QImage(png).scaled(
			pixels,
			pixels,
			Qt::IgnoreAspectRatio,
			Qt::SmoothTransformation));
	} else {
		auto clip = QPainterPath();
		clip.addEllipse(rect);
		p.setClipPath(clip);
		QSvgRenderer(u":/gui/art/ayu/%1/app.svg"_q.arg(id)).render(&p, rect);
	}
}

[[nodiscard]] QImage LoadIconImage(const QString &id, int size) {
	const auto ratio = style::DevicePixelRatio();
	auto result = QImage(
		QSize(size, size) * ratio,
		QImage::Format_ARGB32_Premultiplied);
	result.setDevicePixelRatio(ratio);
	result.fill(Qt::transparent);

	auto p = QPainter(&result);
	auto hq = PainterHighQualityEnabler(p);
	DrawIcon(p, id, QRect(0, 0, size, size), size * ratio);
	p.end();
	return result;
}

// Port of ColorUtils / AppIcons / CubicBezierInterpolator from Android.
struct Hsl {
	float h = 0.f;
	float s = 0.f;
	float l = 0.f;
};

[[nodiscard]] Hsl RgbToHsl(int r, int g, int b) {
	const auto rf = r / 255.f;
	const auto gf = g / 255.f;
	const auto bf = b / 255.f;
	const auto max = std::max(rf, std::max(gf, bf));
	const auto min = std::min(rf, std::min(gf, bf));
	const auto deltaMaxMin = max - min;
	auto h = 0.f;
	auto s = 0.f;
	const auto l = (max + min) / 2.f;
	if (max != min) {
		if (max == rf) {
			h = std::fmod((gf - bf) / deltaMaxMin, 6.f);
		} else if (max == gf) {
			h = ((bf - rf) / deltaMaxMin) + 2.f;
		} else {
			h = ((rf - gf) / deltaMaxMin) + 4.f;
		}
		s = deltaMaxMin / (1.f - std::abs(2.f * l - 1.f));
	}
	h = std::fmod(h * 60.f, 360.f);
	if (h < 0.f) {
		h += 360.f;
	}
	return {
		std::clamp(h, 0.f, 360.f),
		std::clamp(s, 0.f, 1.f),
		std::clamp(l, 0.f, 1.f),
	};
}

[[nodiscard]] QColor HslToColor(Hsl hsl) {
	const auto c = (1.f - std::abs(2.f * hsl.l - 1.f)) * hsl.s;
	const auto m = hsl.l - 0.5f * c;
	const auto x = c * (1.f - std::abs(std::fmod(hsl.h / 60.f, 2.f) - 1.f));
	const auto channel = [](float value) {
		return std::clamp(int(std::lround(255.f * value)), 0, 255);
	};
	switch (int(hsl.h) / 60) {
	case 0: return QColor(channel(c + m), channel(x + m), channel(m));
	case 1: return QColor(channel(x + m), channel(c + m), channel(m));
	case 2: return QColor(channel(m), channel(c + m), channel(x + m));
	case 3: return QColor(channel(m), channel(x + m), channel(c + m));
	case 4: return QColor(channel(x + m), channel(m), channel(c + m));
	case 5:
	case 6: return QColor(channel(c + m), channel(m), channel(x + m));
	}
	return QColor(0, 0, 0);
}

[[nodiscard]] QColor BlendArgb(QColor a, QColor b, float ratio) {
	const auto inverse = 1.f - ratio;
	return QColor(
		int(a.red() * inverse + b.red() * ratio),
		int(a.green() * inverse + b.green() * ratio),
		int(a.blue() * inverse + b.blue() * ratio),
		int(a.alpha() * inverse + b.alpha() * ratio));
}

[[nodiscard]] std::optional<QColor> Dominant(const QImage &image) {
	constexpr auto kBins = 12;
	auto weight = std::array<float, kBins>();
	auto hue = std::array<float, kBins>();
	auto saturation = std::array<float, kBins>();
	auto lightness = std::array<float, kBins>();
	auto colored = 0;
	auto extreme = 0;
	auto extremeLightness = 0.f;
	for (auto y = 0; y != image.height(); ++y) {
		for (auto x = 0; x != image.width(); ++x) {
			const auto pixel = image.pixel(x, y);
			if (qAlpha(pixel) < 200) {
				continue;
			}
			const auto hsl = RgbToHsl(qRed(pixel), qGreen(pixel), qBlue(pixel));
			if (hsl.s < 0.15f) {
				continue;
			}
			if (hsl.l < 0.06f || hsl.l > 0.94f) {
				++extreme;
				extremeLightness += hsl.l;
				continue;
			}
			const auto bin = std::min(kBins - 1, int(hsl.h / 360.f * kBins));
			const auto w = hsl.s * (1.f - std::abs(hsl.l * 2.f - 1.f));
			weight[bin] += w;
			hue[bin] += hsl.h * w;
			saturation[bin] += hsl.s * w;
			lightness[bin] += hsl.l * w;
			++colored;
		}
	}
	auto best = 0;
	for (auto i = 1; i < kBins; ++i) {
		if (weight[i] > weight[best]) {
			best = i;
		}
	}
	if (weight[best] > 0.f && colored >= extreme * 0.15f) {
		return HslToColor({
			hue[best] / weight[best],
			saturation[best] / weight[best],
			lightness[best] / weight[best],
		});
	} else if (!extreme) {
		return std::nullopt;
	}
	return HslToColor({ 0.f, 0.f, extremeLightness / extreme });
}

[[nodiscard]] std::optional<QColor> IconAccent(const QString &id) {
	static auto cache = base::flat_map<QString, std::optional<QColor>>();
	if (const auto i = cache.find(id); i != end(cache)) {
		return i->second;
	}
	auto image = QImage(
		QSize(kAccentSize, kAccentSize),
		QImage::Format_ARGB32_Premultiplied);
	image.fill(Qt::transparent);
	{
		auto p = QPainter(&image);
		auto hq = PainterHighQualityEnabler(p);
		DrawIcon(p, id, QRect(0, 0, kAccentSize, kAccentSize), kAccentSize);
	}
	const auto result = Dominant(
		image.convertToFormat(QImage::Format_ARGB32));
	cache.emplace(id, result);
	return result;
}

[[nodiscard]] std::optional<QColor> IconTint(const QString &id, bool dark) {
	const auto accent = IconAccent(id);
	if (!accent) {
		return std::nullopt;
	}
	auto hsl = RgbToHsl(accent->red(), accent->green(), accent->blue());
	hsl.s = std::min(hsl.s, 0.6f);
	hsl.l = dark ? 0.22f : 0.86f;
	return HslToColor(hsl);
}

[[nodiscard]] float64 EaseOutQuint(float64 time) {
	constexpr auto kStartX = 0.23;
	constexpr auto kStartY = 1.;
	constexpr auto kEndX = 0.32;
	constexpr auto kEndY = 1.;
	const auto cx = 3. * kStartX;
	const auto bx = 3. * (kEndX - kStartX) - cx;
	const auto ax = 1. - cx - bx;
	const auto cy = 3. * kStartY;
	const auto by = 3. * (kEndY - kStartY) - cy;
	const auto ay = 1. - cy - by;
	auto x = time;
	for (auto i = 1; i < 14; ++i) {
		const auto z = x * (cx + x * (bx + x * ax)) - time;
		if (std::abs(z) < 1e-3) {
			break;
		}
		x -= z / (cx + x * (2. * bx + 3. * ax * x));
	}
	return x * (cy + x * (by + x * ay));
}

[[nodiscard]] bool IsDarkTheme() {
	return Ui::CountContrast(st::boxBg->c, st::premiumButtonFg->c)
		> kMinAcceptableContrast;
}

[[nodiscard]] QColor Over(QColor bg, QColor fg) {
	const auto a = fg.alphaF();
	return QColor::fromRgbF(
		bg.redF() * (1. - a) + fg.redF() * a,
		bg.greenF() * (1. - a) + fg.greenF() * a,
		bg.blueF() * (1. - a) + fg.blueF() * a);
}

[[nodiscard]] QColor GrayColor(bool dark) {
	if (!dark) {
		return st::boxDividerBg->c;
	}
	// WHY: TopBarAbstract::paintEdges darkens boxBg with shadowFg twice
	// in dark themes, the same look Android gets from windowBackgroundGray.
	return Over(Over(st::boxBg->c, st::shadowFg->c), st::shadowFg->c);
}

[[nodiscard]] float64 TextProgress(float64 collapse) {
	return std::min(1., collapse / kTextCollapsePart);
}

struct PageState {
	rpl::variable<QString> selected;
	rpl::variable<int> tab = 0;
	rpl::variable<float64> collapse = 0.;
	std::optional<QColor> tint;
	Ui::Animations::Simple tintAnimation;
	rpl::event_stream<> repaints;
	float64 fade = 0.;
	bool dark = false;
};

[[nodiscard]] QColor BaseColor(const PageState &state) {
	const auto gray = GrayColor(state.dark);
	return state.tint ? BlendArgb(gray, *state.tint, 0.25f) : gray;
}

[[nodiscard]] QBrush GlowBrush(QColor tint, QPointF center, float64 radius) {
	auto from = tint;
	from.setAlpha(200);
	auto to = tint;
	to.setAlpha(0);
	auto gradient = QRadialGradient(center, radius);
	gradient.setStops({ { 0., from }, { 1., to } });
	return QBrush(gradient);
}

void AnimateTint(not_null<PageState*> state, std::optional<QColor> to) {
	state->tintAnimation.stop();
	if (state->tint == to) {
		return;
	}
	const auto from = state->tint;
	if (!from || !to) {
		state->tint = to;
		state->repaints.fire({});
		return;
	}
	state->tintAnimation.start([=](float64 value) {
		state->tint = BlendArgb(*from, *to, float(value));
		state->repaints.fire({});
	}, 0., 1., kTintDuration, [](float64 delta, float64 dt) {
		return delta * EaseOutQuint(dt);
	});
}

void ApplyIcon(const QString &id) {
	auto &settings = AyuSettings::getInstance();
	if (settings.appIcon() == id) {
		return;
	}
	settings.setAppIcon(id);

#ifdef Q_OS_WIN
	AyuAssets::loadAppIco();
	reloadAppIconFromTaskBar();
#endif

	Window::OverrideApplicationIcon(AyuAssets::currentAppLogo());
	Core::App().refreshApplicationIcon();
	Core::App().tray().updateIconCounters();
	Core::App().domain().notifyUnreadBadgeChanged();
}

class IconsGrid final : public Ui::RpWidget {
public:
	IconsGrid(
		QWidget *parent,
		std::vector<IconInfo> icons,
		not_null<rpl::variable<QString>*> selected);

protected:
	int resizeGetHeight(int newWidth) override;
	void paintEvent(QPaintEvent *e) override;
	void mousePressEvent(QMouseEvent *e) override;
	void mouseReleaseEvent(QMouseEvent *e) override;

private:
	[[nodiscard]] int selectionSize() const;
	[[nodiscard]] int rowHeight() const;
	[[nodiscard]] int cellLeft(int column) const;
	[[nodiscard]] int indexAt(QPoint point) const;

	const std::vector<IconInfo> _icons;
	std::vector<QImage> _images;
	std::vector<Ui::Text::String> _names;
	const not_null<rpl::variable<QString>*> _selected;
	int _columns = 4;
	int _pressed = -1;

};

IconsGrid::IconsGrid(
	QWidget *parent,
	std::vector<IconInfo> icons,
	not_null<rpl::variable<QString>*> selected)
: RpWidget(parent)
, _icons(std::move(icons))
, _selected(selected) {
	_images.reserve(_icons.size());
	_names.reserve(_icons.size());
	for (const auto &icon : _icons) {
		_images.push_back(AyuAssets::loadPreview(icon.id));
		_names.emplace_back(st::defaultTextStyle, IconName(icon.id));
	}
	setCursor(style::cur_pointer);
	_selected->changes() | rpl::on_next([=] {
		update();
	}, lifetime());
}

int IconsGrid::selectionSize() const {
	return st::iconPickerIconSize + st::iconPickerSelectedPadding * 2;
}

int IconsGrid::rowHeight() const {
	return selectionSize()
		+ st::appIconPageCaptionSkip
		+ st::normalFont->height * kCaptionLines;
}

int IconsGrid::cellLeft(int column) const {
	return (width() * column) / _columns;
}

int IconsGrid::resizeGetHeight(int newWidth) {
	const auto count = std::clamp(
		int(std::lround(newWidth / float64(st::appIconPageColumnWidth))),
		kMinColumns,
		kMaxColumns);
	_columns = (count < kMaxColumns
		&& newWidth / float64(count) > st::appIconPageColumnMaxWidth)
		? (count + 1)
		: count;
	const auto rows = (int(_icons.size()) + _columns - 1) / _columns;
	return rows
		? (rows * rowHeight() + (rows - 1) * st::appIconPageRowSkip)
		: 0;
}

int IconsGrid::indexAt(QPoint point) const {
	const auto step = rowHeight() + st::appIconPageRowSkip;
	if (point.y() < 0 || point.x() < 0 || point.x() >= width()) {
		return -1;
	}
	const auto row = point.y() / step;
	if (point.y() - row * step >= rowHeight()) {
		return -1;
	}
	for (auto column = 0; column != _columns; ++column) {
		if (point.x() >= cellLeft(column) && point.x() < cellLeft(column + 1)) {
			const auto index = row * _columns + column;
			return (index < int(_icons.size())) ? index : -1;
		}
	}
	return -1;
}

void IconsGrid::paintEvent(QPaintEvent *) {
	auto p = Painter(this);
	const auto size = selectionSize();
	const auto padding = st::iconPickerSelectedPadding;
	const auto radius = st::iconPickerSelectedRounding;
	const auto step = rowHeight() + st::appIconPageRowSkip;
	for (auto i = 0, count = int(_icons.size()); i != count; ++i) {
		const auto column = i % _columns;
		const auto left = cellLeft(column);
		const auto cellWidth = cellLeft(column + 1) - left;
		const auto top = (i / _columns) * step;
		const auto x = left + (cellWidth - size) / 2;
		if (_icons[i].id == _selected->current()) {
			auto hq = PainterHighQualityEnabler(p);
			p.setPen(Qt::NoPen);
			p.setBrush(st::boxDividerBg);
			p.drawRoundedRect(QRect(x, top, size, size), radius, radius);
		}
		p.drawImage(x + padding, top + padding, _images[i]);
		p.setPen(st::windowFg);
		_names[i].drawElided(
			p,
			left,
			top + size + st::appIconPageCaptionSkip,
			cellWidth,
			kCaptionLines,
			style::al_top);
	}
}

void IconsGrid::mousePressEvent(QMouseEvent *e) {
	_pressed = (e->button() == Qt::LeftButton) ? indexAt(e->pos()) : -1;
}

void IconsGrid::mouseReleaseEvent(QMouseEvent *e) {
	const auto pressed = std::exchange(_pressed, -1);
	if (e->button() == Qt::LeftButton
		&& pressed >= 0
		&& pressed == indexAt(e->pos())) {
		*_selected = _icons[pressed].id;
	}
}

[[nodiscard]] int HeaderHeight() {
	return st::infoLayerTopBarHeight;
}

[[nodiscard]] int HeroBodyHeight() {
	return st::appIconPagePreviewTop
		+ st::appIconPagePreviewSize
		+ st::appIconPageNameSkip
		+ st::appIconPageNameHeight
		+ st::appIconPageSubtitleSkip
		+ st::appIconPageSubtitleHeight
		+ st::appIconPageHeroBottom;
}

[[nodiscard]] float64 GlowCenterY() {
	return HeaderHeight()
		+ st::appIconPagePreviewTop
		+ st::appIconPagePreviewSize / 2.;
}

[[nodiscard]] float64 GlowRadius(int width) {
	return std::max(width, HeaderHeight() + HeroBodyHeight()) * kGlowRadius;
}

void PaintHero(
		QPainter &p,
		const PageState &state,
		const QPainterPath &area,
		int width,
		int top) {
	p.fillPath(area, BaseColor(state));
	const auto collapse = state.collapse.current();
	if (state.tint && collapse < 1.) {
		p.setOpacity(1. - collapse);
		p.fillPath(area, GlowBrush(
			*state.tint,
			QPointF(width / 2., GlowCenterY() - top),
			GlowRadius(width)));
		p.setOpacity(1.);
	}
}

class AppIconHeader final : public Ui::RpWidget {
public:
	AppIconHeader(QWidget *parent, std::shared_ptr<PageState> state);

	void setRoundEdges(bool value);

protected:
	void paintEvent(QPaintEvent *e) override;

private:
	const std::shared_ptr<PageState> _state;
	bool _roundEdges = false;

};

AppIconHeader::AppIconHeader(
	QWidget *parent,
	std::shared_ptr<PageState> state)
: RpWidget(parent)
, _state(std::move(state)) {
	setMinimumHeight(HeaderHeight());
	setMaximumHeight(HeaderHeight());
	resize(width(), HeaderHeight());

	rpl::merge(
		_state->repaints.events(),
		_state->collapse.changes() | rpl::to_empty
	) | rpl::on_next([=] {
		update();
	}, lifetime());
}

void AppIconHeader::setRoundEdges(bool value) {
	_roundEdges = value;
	update();
}

void AppIconHeader::paintEvent(QPaintEvent *e) {
	auto p = QPainter(this);
	auto hq = PainterHighQualityEnabler(p);
	auto area = QPainterPath();
	if (_roundEdges) {
		const auto radius = st::boxRadius;
		area.addRoundedRect(
			QRectF(rect() + QMargins{ 0, 0, 0, radius + 1 }),
			radius,
			radius);
	} else {
		area.addRect(QRectF(rect()));
	}
	PaintHero(p, *_state, area, width(), y());
}

class AppIconSection;

} // namespace

template <>
struct SectionFactory<AppIconSection> : AbstractSectionFactory {
	object_ptr<AbstractSection> create(
		not_null<QWidget*> parent,
		not_null<Window::SessionController*> controller,
		not_null<Ui::ScrollArea*> scroll,
		rpl::producer<Container> containerValue
	) const final override;
	bool hasCustomTopBar() const final override {
		return true;
	}

	[[nodiscard]] static const std::shared_ptr<SectionFactory> &Instance() {
		static const auto result = std::make_shared<SectionFactory>();
		return result;
	}
};

namespace {

class AppIconSection final : public Section<AppIconSection> {
public:
	AppIconSection(
		QWidget *parent,
		not_null<Window::SessionController*> controller,
		not_null<Ui::ScrollArea*> scroll);

	[[nodiscard]] rpl::producer<QString> title() override;

	[[nodiscard]] base::weak_qptr<Ui::RpWidget> createPinnedToTop(
		not_null<QWidget*> parent) override;

	[[nodiscard]] bool hasFlexibleTopBar() const override;

	void setStepDataReference(std::any &data) override;

	[[nodiscard]] rpl::producer<> sectionShowBack() override final;

protected:
	void paintEvent(QPaintEvent *e) override;

private:
	void setupContent();
	void setupScroll();
	void setupLabels();
	void setupEdge();
	void setupHero(
		not_null<QWidget*> parent,
		not_null<AppIconHeader*> header);
	void setupTabs(not_null<QWidget*> parent);
	[[nodiscard]] const style::color &panelBg() const;

	const not_null<Ui::ScrollArea*> _scroll;
	const std::shared_ptr<PageState> _state;
	base::unique_qptr<Ui::FadeWrap<Ui::IconButton>> _back;
	base::unique_qptr<Ui::IconButton> _close;
	base::unique_qptr<Ui::FlatLabel> _title;
	base::unique_qptr<Ui::RpWidget> _heroBack;
	base::unique_qptr<Ui::RpWidget> _heroPreview;
	base::unique_qptr<Info::Profile::TabsStrip> _tabs;
	rpl::variable<int> _scrollTop;
	rpl::variable<bool> _backToggles;
	rpl::variable<Info::Wrap> _wrap;
	rpl::event_stream<> _showBack;

};

AppIconSection::AppIconSection(
	QWidget *parent,
	not_null<Window::SessionController*> controller,
	not_null<Ui::ScrollArea*> scroll)
: Section(parent, controller)
, _scroll(scroll)
, _state(std::make_shared<PageState>()) {
	const auto current = CurrentIcon();
	_state->selected = current;
	_state->tab = GroupIndex(current);
	_state->dark = IsDarkTheme();
	_state->tint = IconTint(current, _state->dark);

	const auto state = _state.get();
	_state->selected.changes() | rpl::on_next([=](const QString &id) {
		ApplyIcon(id);
		AnimateTint(state, IconTint(id, state->dark));
	}, lifetime());

	style::PaletteChanged() | rpl::on_next([=] {
		state->tintAnimation.stop();
		state->dark = IsDarkTheme();
		state->tint = IconTint(state->selected.current(), state->dark);
		state->repaints.fire({});
	}, lifetime());

	setupContent();
	setupScroll();
	setupLabels();
	setupEdge();
}

rpl::producer<QString> AppIconSection::title() {
	return tr::ayu_AppIconHeader();
}

bool AppIconSection::hasFlexibleTopBar() const {
	return true;
}

rpl::producer<> AppIconSection::sectionShowBack() {
	return _showBack.events();
}

void AppIconSection::setStepDataReference(std::any &data) {
	const auto my = std::any_cast<SectionCustomTopBarData>(&data);
	if (my) {
		_backToggles = std::move(
			my->backButtonEnables
		) | rpl::map_to(true);
		_wrap = std::move(my->wrapValue);
	}
}

const style::color &AppIconSection::panelBg() const {
	return (_wrap.current() == Info::Wrap::Layer)
		? st::boxBg
		: st::profileBg;
}

void AppIconSection::paintEvent(QPaintEvent *e) {
	auto p = QPainter(this);
	const auto top = HeroBodyHeight();
	p.fillRect(
		QRect(0, top, width(), height() - top).intersected(e->rect()),
		panelBg());
}

void AppIconSection::setupContent() {
	const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);
	Ui::AddSkip(content, HeroBodyHeight() + st::appIconPageGridTop);
	const auto &groups = Groups();
	for (auto i = 0, count = int(groups.size()); i != count; ++i) {
		const auto wrap = content->add(
			object_ptr<Ui::SlideWrap<IconsGrid>>(
				content,
				object_ptr<IconsGrid>(
					content,
					groups[i].icons,
					&_state->selected),
				st::appIconPageGridPadding));
		wrap->toggleOn(_state->tab.value() | rpl::map([=](int tab) {
			return (tab == i);
		}), anim::type::instant);
	}
	Ui::AddSkip(content, st::appIconPageGridBottom);
	Ui::ResizeFitChild(this, content);

	_wrap.changes() | rpl::on_next([=] {
		update();
	}, lifetime());
}

void AppIconSection::setupScroll() {
	_scrollTop = _scroll->scrollTopValue();
	_scrollTop.value() | rpl::on_next([=](int top) {
		_state->fade = std::min(
			1.,
			top / float64(st::appIconPageFadeDistance));
		_state->collapse = std::clamp(top / float64(HeroBodyHeight()), 0., 1.);
	}, lifetime());

	struct WheelState {
		Ui::Animations::Basic animation;
		int fromTop = 0;
		int targetTop = 0;
		crl::time startTime = 0;
	};
	const auto scroll = _scroll;
	const auto wheel = scroll->lifetime().make_state<WheelState>();
	const auto single = scroll->verticalScrollBar()->singleStep()
		* QApplication::wheelScrollLines();
	// WHY: a wheel notch jumps a whole step at once; the profile animates
	// it in info_flexible_scroll.cpp, copied here without its cover-specific
	// snap steps so the collapse follows the scroll as smoothly as a finger.
	wheel->animation.init([=](crl::time now) {
		const auto progress = std::clamp(
			(now - wheel->startTime) / float64(kScrollStepTime),
			0.,
			1.);
		scroll->scrollToY(anim::interpolate(
			wheel->fromTop,
			wheel->targetTop,
			anim::easeOutQuint(1., progress)));
		if (progress >= 1.) {
			wheel->animation.stop();
		}
	});
	scroll->setCustomWheelProcess([=](not_null<QWheelEvent*> e) {
		const auto delta = e->angleDelta().y();
		if (std::abs(delta) != 120 || e->phase() != Qt::NoScrollPhase) {
			wheel->animation.stop();
			return false;
		}
		const auto base = wheel->animation.animating()
			? wheel->targetTop
			: scroll->scrollTop();
		const auto diff = (delta > 0) ? -single : single;
		wheel->targetTop = std::clamp(base + diff, 0, scroll->scrollTopMax());
		wheel->fromTop = scroll->scrollTop();
		wheel->startTime = crl::now();
		if (!wheel->animation.animating()) {
			wheel->animation.start();
		}
		return true;
	});
}

void AppIconSection::setupLabels() {
	const auto labels = Ui::CreateChild<Ui::RpWidget>(this);
	const auto name = Ui::CreateChild<Ui::FlatLabel>(
		labels,
		_state->selected.value() | rpl::map(IconName),
		st::appIconPageName);
	const auto subtitle = Ui::CreateChild<Ui::FlatLabel>(
		labels,
		_state->selected.value() | rpl::map(IconSubtitle),
		st::appIconPageSubtitle);

	rpl::combine(
		_scrollTop.value(),
		widthValue(),
		name->heightValue(),
		subtitle->heightValue()
	) | rpl::on_next([=](int top, int width, int, int) {
		const auto visible = std::max(HeroBodyHeight() - top, 0);
		labels->setGeometry(0, top, width, visible);
		labels->setVisible(visible > 0);

		const auto text = TextProgress(_state->collapse.current());
		const auto shift = int(std::round(-st::appIconPageTextShift * text));
		const auto padding = st::appIconPageTextPadding;
		const auto available = std::max(width - 2 * padding, 1);
		const auto nameTop = st::appIconPagePreviewTop
			+ st::appIconPagePreviewSize
			+ st::appIconPageNameSkip;
		const auto subtitleTop = nameTop
			+ st::appIconPageNameHeight
			+ st::appIconPageSubtitleSkip;
		const auto place = [&](
				not_null<Ui::FlatLabel*> label,
				int labelTop,
				int height) {
			if (label->width() != available) {
				label->resizeToWidth(available);
			}
			label->moveToLeft(
				padding,
				labelTop + (height - label->height()) / 2 + shift);
			label->setOpacity(1. - text);
			label->setVisible(text < 1.);
		};
		place(name, nameTop, st::appIconPageNameHeight);
		place(subtitle, subtitleTop, st::appIconPageSubtitleHeight);
	}, labels->lifetime());
}

void AppIconSection::setupEdge() {
	const auto edge = Ui::CreateChild<Ui::RpWidget>(this);
	edge->setAttribute(Qt::WA_TransparentForMouseEvents);

	rpl::combine(
		_scrollTop.value(),
		widthValue()
	) | rpl::on_next([=](int top, int width) {
		edge->setGeometry(
			0,
			std::max(HeroBodyHeight(), top),
			width,
			std::max(st::appIconPagePanelRadius, st::appIconPageFadeHeight));
		edge->raise();
		edge->update();
	}, edge->lifetime());

	_state->repaints.events(
	) | rpl::on_next([=] {
		edge->update();
	}, edge->lifetime());

	edge->paintRequest(
	) | rpl::on_next([=] {
		auto p = QPainter(edge);
		auto hq = PainterHighQualityEnabler(p);
		const auto radius = st::appIconPagePanelRadius;
		const auto width = edge->width();
		auto panel = QPainterPath();
		panel.addRoundedRect(
			QRectF(0, 0, width, edge->height() + radius),
			radius,
			radius);
		auto corners = QPainterPath();
		corners.addRect(QRectF(0, 0, width, radius));
		corners = corners.subtracted(panel);

		const auto top = HeaderHeight() + edge->y() - _scrollTop.current();
		PaintHero(p, *_state, corners, width, top);

		if (_state->fade > 0.) {
			const auto height = st::appIconPageFadeHeight;
			auto from = panelBg()->c;
			auto to = from;
			to.setAlpha(0);
			auto gradient = QLinearGradient(0, 0, 0, height);
			gradient.setStops({ { 0., from }, { 1., to } });
			p.setClipPath(panel);
			p.setOpacity(_state->fade);
			p.fillRect(QRect(0, 0, width, height), QBrush(gradient));
		}
	}, edge->lifetime());
}

base::weak_qptr<Ui::RpWidget> AppIconSection::createPinnedToTop(
		not_null<QWidget*> parent) {
	const auto content = Ui::CreateChild<AppIconHeader>(
		parent.get(),
		_state);

	_wrap.value(
	) | rpl::on_next([=](Info::Wrap wrap) {
		content->setRoundEdges(wrap == Info::Wrap::Layer);
	}, content->lifetime());

	_wrap.value(
	) | rpl::on_next([=](Info::Wrap wrap) {
		const auto isLayer = (wrap == Info::Wrap::Layer);
		const auto &topBar = isLayer ? st::infoLayerTopBar : st::infoTopBar;
		_title = base::make_unique_q<Ui::FlatLabel>(
			content,
			tr::ayu_AppIconHeader(),
			topBar.title);
		_title->setAttribute(Qt::WA_TransparentForMouseEvents);
		_state->collapse.value(
		) | rpl::on_next([=](float64 collapse) {
			_title->setOpacity(1. - TextProgress(collapse));
		}, _title->lifetime());

		_back = base::make_unique_q<Ui::FadeWrap<Ui::IconButton>>(
			content,
			object_ptr<Ui::IconButton>(content, topBar.back),
			st::infoTopBarScale);
		_back->setDuration(0);
		_back->toggleOn(isLayer
			? _backToggles.value() | rpl::type_erased
			: rpl::single(true));
		_back->entity()->addClickHandler([=] {
			_showBack.fire({});
		});
		_back->toggledValue(
		) | rpl::on_next([=](bool toggled) {
			const auto &st = isLayer ? st::infoLayerTopBar : st::infoTopBar;
			_title->moveToLeft(
				toggled ? st.back.width : st.titlePosition.x(),
				st.titlePosition.y());
		}, _back->lifetime());

		if (!isLayer) {
			_close = nullptr;
		} else {
			_close = base::make_unique_q<Ui::IconButton>(
				content,
				st::infoLayerTopBarClose);
			_close->addClickHandler([=] {
				controller()->parentController()->hideLayer();
				controller()->parentController()->hideSpecialLayer();
			});
			content->widthValue(
			) | rpl::on_next([=] {
				_close->moveToRight(0, 0);
			}, _close->lifetime());
		}
	}, content->lifetime());

	setupHero(parent, content);
	setupTabs(parent);

	return base::make_weak(not_null<Ui::RpWidget*>{ content });
}

void AppIconSection::setupHero(
		not_null<QWidget*> parent,
		not_null<AppIconHeader*> header) {
	_heroBack = base::make_unique_q<Ui::RpWidget>(parent.get());
	const auto back = _heroBack.get();
	back->setAttribute(Qt::WA_TransparentForMouseEvents);
	back->paintRequest(
	) | rpl::on_next([=] {
		auto p = QPainter(back);
		auto area = QPainterPath();
		area.addRect(QRectF(back->rect()));
		PaintHero(p, *_state, area, back->width(), back->y());
	}, back->lifetime());

	_heroPreview = base::make_unique_q<Ui::RpWidget>(parent.get());
	const auto preview = _heroPreview.get();
	preview->setAttribute(Qt::WA_TransparentForMouseEvents);
	const auto image = preview->lifetime().make_state<QImage>();
	_state->selected.value(
	) | rpl::on_next([=](const QString &id) {
		*image = LoadIconImage(id, st::appIconPagePreviewSize);
		preview->update();
	}, preview->lifetime());
	preview->paintRequest(
	) | rpl::on_next([=] {
		auto p = QPainter(preview);
		auto hq = PainterHighQualityEnabler(p);
		const auto collapse = _state->collapse.current();
		const auto full = float64(st::appIconPagePreviewSize);
		const auto size = full
			+ (st::appIconPagePreviewCollapsed - full) * collapse;
		const auto from = GlowCenterY();
		const auto to = HeaderHeight() / 2.;
		const auto center = QPointF(
			preview->width() / 2.,
			from + (to - from) * collapse);
		p.drawImage(
			QRectF(
				center.x() - size / 2.,
				center.y() - size / 2.,
				size,
				size),
			*image);
	}, preview->lifetime());

	rpl::merge(
		_state->repaints.events(),
		_state->collapse.changes() | rpl::to_empty
	) | rpl::on_next([=] {
		back->update();
		preview->update();
	}, preview->lifetime());

	header->widthValue(
	) | rpl::on_next([=](int width) {
		back->setGeometry(0, HeaderHeight(), width, HeroBodyHeight());
		back->lower();
		preview->setGeometry(0, 0, width, HeaderHeight() + HeroBodyHeight());
		preview->raise();
	}, preview->lifetime());
}

void AppIconSection::setupTabs(not_null<QWidget*> parent) {
	_tabs = base::make_unique_q<Info::Profile::TabsStrip>(
		parent.get(),
		st::infoProfileTabsStrip);
	const auto tabs = _tabs.get();

	const auto &groups = Groups();
	auto list = std::vector<Info::Profile::StripTab>();
	list.reserve(groups.size());
	for (auto i = 0, count = int(groups.size()); i != count; ++i) {
		list.push_back({
			.id = QString::number(i),
			.text = { groups[i].title },
		});
	}
	tabs->setTabs(std::move(list));
	tabs->setActiveTab(QString::number(_state->tab.current()));

	tabs->activated(
	) | rpl::on_next([=](const QString &id) {
		tabs->setActiveTab(id);
		_state->tab = id.toInt();
	}, tabs->lifetime());

	// WHY: the strip floats over the list instead of living inside it,
	// so a wheel it doesn't need would never reach the scroll; forward it
	// like info_settings_widget.cpp does for the pinned top bar.
	tabs->events(
	) | rpl::filter([=](not_null<QEvent*> e) {
		return (e->type() == QEvent::Wheel)
			&& (tabs->width() >= tabs->naturalWidth());
	}) | rpl::on_next([=](not_null<QEvent*> e) {
		_scroll->viewportEvent(e);
	}, tabs->lifetime());

	const auto outer = dynamic_cast<Ui::RpWidget*>(parent.get());
	if (!outer) {
		return;
	}
	outer->sizeValue(
	) | rpl::on_next([=](QSize size) {
		tabs->resizeToWidth(std::min(tabs->naturalWidth(), size.width()));
		tabs->moveToLeft(
			(size.width() - tabs->width()) / 2,
			size.height() - tabs->height() - st::appIconPageTabsSkip);
		tabs->raise();
	}, tabs->lifetime());
}

} // namespace

object_ptr<AbstractSection> SectionFactory<AppIconSection>::create(
		not_null<QWidget*> parent,
		not_null<Window::SessionController*> controller,
		not_null<Ui::ScrollArea*> scroll,
		rpl::producer<Container> containerValue) const {
	return object_ptr<AppIconSection>(parent, controller, scroll);
}

Type AppIconId() {
	return AppIconSection::Id();
}

} // namespace Settings

object_ptr<Ui::RpWidget> CreateAppIconButton(
		not_null<Ui::VerticalLayout*> container,
		Fn<void(Settings::Type)> showOther) {
	auto result = Settings::CreateButtonWithIcon(
		container,
		tr::ayu_AppIconHeader(),
		st::appIconSettingsButton,
		{ &st::menuIconDraw });
	const auto button = result.data();

	const auto current = button->lifetime().make_state<
		rpl::variable<QString>
	>(AyuSettings::getInstance().appIconValue(
	) | rpl::map([](const QString &) {
		return Settings::CurrentIcon();
	}));

	const auto subtitle = Ui::CreateChild<Ui::FlatLabel>(
		button,
		current->value() | rpl::map(Settings::IconName),
		st::appIconSettingsSubtitle);
	subtitle->setAttribute(Qt::WA_TransparentForMouseEvents);

	const auto size = st::appIconSettingsPreviewSize;
	const auto preview = Ui::CreateChild<Ui::RpWidget>(button);
	preview->setAttribute(Qt::WA_TransparentForMouseEvents);
	preview->resize(size, size);
	const auto image = preview->lifetime().make_state<QImage>();
	current->value() | rpl::on_next([=](const QString &id) {
		*image = Settings::LoadIconImage(id, size);
		preview->update();
	}, preview->lifetime());
	preview->paintRequest() | rpl::on_next([=] {
		auto p = QPainter(preview);
		p.drawImage(0, 0, *image);
	}, preview->lifetime());

	button->sizeValue() | rpl::on_next([=](QSize outer) {
		const auto &padding = st::appIconSettingsButton.padding;
		subtitle->resizeToWidth(
			outer.width() - padding.left() - padding.right());
		subtitle->moveToLeft(padding.left(), st::appIconSettingsSubtitleTop);
		preview->moveToRight(
			st::appIconSettingsPreviewRight,
			(outer.height() - size) / 2);
	}, button->lifetime());

	button->addClickHandler([=] {
		if (showOther) {
			showOther(Settings::AppIconId());
		}
	});
	return object_ptr<Ui::RpWidget>(std::move(result));
}
