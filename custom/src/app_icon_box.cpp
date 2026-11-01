#include "ayu/ui/boxes/app_icon_box.h"

#include "lang_auto.h"
#include "tray.h"
#include "ayu/ayu_settings.h"
#include "ayu/ui/ayu_logo.h"
#include "core/application.h"
#include "lang/lang_keys.h"
#include "main/main_domain.h"
#include "settings/settings_common.h"
#include "styles/style_app_icon_box.h"
#include "styles/style_ayu_styles.h"
#include "styles/style_layers.h"
#include "styles/style_menu_icons.h"
#include "styles/style_settings.h"
#include "ui/layers/generic_box.h"
#include "ui/painter.h"
#include "ui/text/text_utilities.h"
#include "ui/ui_utility.h"
#include "ui/vertical_list.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/labels.h"
#include "ui/wrap/padding_wrap.h"
#include "ui/wrap/vertical_layout.h"
#include "window/main_window.h"
#include "window/window_session_controller.h"

#ifdef Q_OS_WIN
#include "ayu/utils/windows_utils.h"
#endif

#include <QSvgRenderer>

namespace {

constexpr auto kColumns = 4;
constexpr auto kCaptionLines = 2;

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

[[nodiscard]] QImage LoadIconImage(const QString &id, int size) {
	const auto ratio = style::DevicePixelRatio();
	auto result = QImage(
		QSize(size, size) * ratio,
		QImage::Format_ARGB32_Premultiplied);
	result.setDevicePixelRatio(ratio);
	result.fill(Qt::transparent);

	auto p = QPainter(&result);
	auto hq = PainterHighQualityEnabler(p);
	const auto rect = QRect(0, 0, size, size);
	const auto png = u":/gui/art/ayu/%1/app.png"_q.arg(id);
	if (QFile::exists(png)) {
		p.drawImage(rect, QImage(png).scaled(
			rect.size() * ratio,
			Qt::IgnoreAspectRatio,
			Qt::SmoothTransformation));
	} else {
		auto clip = QPainterPath();
		clip.addEllipse(rect);
		p.setClipPath(clip);
		QSvgRenderer(u":/gui/art/ayu/%1/app.svg"_q.arg(id)).render(&p, rect);
	}
	p.end();
	return result;
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

	[[nodiscard]] int rowTop(const QString &id) const;
	[[nodiscard]] int rowHeight() const;

protected:
	int resizeGetHeight(int newWidth) override;
	void paintEvent(QPaintEvent *e) override;
	void mousePressEvent(QMouseEvent *e) override;

private:
	[[nodiscard]] int selectionSize() const;
	[[nodiscard]] int cellLeft(int column) const;
	[[nodiscard]] int indexAt(QPoint point) const;

	const std::vector<IconInfo> _icons;
	std::vector<QImage> _images;
	std::vector<Ui::Text::String> _names;
	const not_null<rpl::variable<QString>*> _selected;

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
		+ st::appIconBoxCaptionSkip
		+ st::normalFont->height * kCaptionLines;
}

int IconsGrid::rowTop(const QString &id) const {
	for (auto i = 0, count = int(_icons.size()); i != count; ++i) {
		if (_icons[i].id == id) {
			return (i / kColumns) * (rowHeight() + st::appIconBoxRowSkip);
		}
	}
	return -1;
}

int IconsGrid::cellLeft(int column) const {
	return (width() * column) / kColumns;
}

int IconsGrid::resizeGetHeight(int) {
	const auto rows = (int(_icons.size()) + kColumns - 1) / kColumns;
	return rows
		? (rows * rowHeight() + (rows - 1) * st::appIconBoxRowSkip)
		: 0;
}

int IconsGrid::indexAt(QPoint point) const {
	const auto step = rowHeight() + st::appIconBoxRowSkip;
	const auto row = point.y() / step;
	if (point.y() < 0 || point.y() - row * step >= rowHeight()) {
		return -1;
	}
	for (auto column = 0; column != kColumns; ++column) {
		if (point.x() >= cellLeft(column) && point.x() < cellLeft(column + 1)) {
			const auto index = row * kColumns + column;
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
	const auto step = rowHeight() + st::appIconBoxRowSkip;
	for (auto i = 0, count = int(_icons.size()); i != count; ++i) {
		const auto column = i % kColumns;
		const auto left = cellLeft(column);
		const auto cellWidth = cellLeft(column + 1) - left;
		const auto top = (i / kColumns) * step;
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
			top + size + st::appIconBoxCaptionSkip,
			cellWidth,
			kCaptionLines,
			style::al_top);
	}
}

void IconsGrid::mousePressEvent(QMouseEvent *e) {
	if (e->button() != Qt::LeftButton) {
		return;
	}
	const auto index = indexAt(e->pos());
	if (index >= 0) {
		*_selected = _icons[index].id;
	}
}

} // namespace

void AppIconBox(not_null<Ui::GenericBox*> box) {
	box->setTitle(tr::ayu_AppIconHeader());
	box->setWidth(st::boxWideWidth);
	box->setMaxHeight(st::appIconBoxMaxHeight);

	const auto selected = box->lifetime().make_state<rpl::variable<QString>>(
		CurrentIcon());

	auto top = object_ptr<Ui::VerticalLayout>(box);
	const auto preview = top->add(
		object_ptr<Ui::RpWidget>(top.data()),
		style::margins(0, st::appIconBoxPreviewTop, 0, 0));
	const auto previewSize = st::appIconBoxPreviewSize;
	preview->resize(previewSize, previewSize);
	const auto previewImage = preview->lifetime().make_state<QImage>();
	selected->value() | rpl::on_next([=](const QString &id) {
		*previewImage = LoadIconImage(id, previewSize);
		preview->update();
	}, preview->lifetime());
	preview->paintRequest() | rpl::on_next([=] {
		auto p = QPainter(preview);
		p.drawImage((preview->width() - previewSize) / 2, 0, *previewImage);
	}, preview->lifetime());

	top->add(
		object_ptr<Ui::FlatLabel>(
			top.data(),
			selected->value() | rpl::map(IconName),
			st::appIconBoxName),
		st::appIconBoxNamePadding);

	const auto subtitle = top->add(
		object_ptr<Ui::FixedHeightWidget>(
			top.data(),
			st::appIconBoxSubtitleHeight),
		st::appIconBoxSubtitlePadding);
	const auto subtitleLabel = Ui::CreateChild<Ui::FlatLabel>(
		subtitle,
		selected->value() | rpl::map(IconSubtitle),
		st::appIconBoxSubtitle);
	subtitle->widthValue() | rpl::on_next([=](int width) {
		subtitleLabel->resizeToWidth(width);
		subtitleLabel->moveToLeft(0, 0);
	}, subtitleLabel->lifetime());
	box->setPinnedToTopContent(std::move(top));

	const auto content = box->verticalLayout();
	auto selectedGrid = (IconsGrid*)nullptr;
	auto first = true;
	for (const auto &group : Groups()) {
		if (!first) {
			Ui::AddSkip(content, st::appIconBoxGroupSkip);
		}
		first = false;
		Ui::AddSubsectionTitle(content, rpl::single(group.title));
		const auto grid = content->add(
			object_ptr<IconsGrid>(content, group.icons, selected),
			st::appIconBoxGridPadding);
		if (grid->rowTop(selected->current()) >= 0) {
			selectedGrid = grid;
		}
	}
	Ui::AddSkip(content);

	if (selectedGrid) {
		box->setInitScrollCallback([=] {
			const auto scrollTop = selectedGrid->y()
				+ selectedGrid->rowTop(selected->current());
			box->scrollToY(scrollTop, scrollTop + selectedGrid->rowHeight());
		});
	}

	box->addButton(tr::lng_settings_save(), [=] {
		const auto id = selected->current();
		box->closeBox();
		ApplyIcon(id);
	});
	box->addButton(tr::lng_cancel(), [=] {
		box->closeBox();
	});
}

object_ptr<Ui::RpWidget> CreateAppIconButton(
		not_null<Ui::VerticalLayout*> container,
		not_null<Window::SessionController*> controller) {
	auto result = Settings::CreateButtonWithIcon(
		container,
		tr::ayu_AppIconHeader(),
		st::appIconSettingsButton,
		{ &st::menuIconPalette });
	const auto button = result.data();

	const auto current = button->lifetime().make_state<
		rpl::variable<QString>
	>(AyuSettings::getInstance().appIconValue(
	) | rpl::map([](const QString &) {
		return CurrentIcon();
	}));

	const auto subtitle = Ui::CreateChild<Ui::FlatLabel>(
		button,
		current->value() | rpl::map(IconName),
		st::appIconSettingsSubtitle);
	subtitle->setAttribute(Qt::WA_TransparentForMouseEvents);

	const auto size = st::appIconSettingsPreviewSize;
	const auto preview = Ui::CreateChild<Ui::RpWidget>(button);
	preview->setAttribute(Qt::WA_TransparentForMouseEvents);
	preview->resize(size, size);
	const auto image = preview->lifetime().make_state<QImage>();
	current->value() | rpl::on_next([=](const QString &id) {
		*image = LoadIconImage(id, size);
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
		controller->show(Box(AppIconBox));
	});
	return object_ptr<Ui::RpWidget>(std::move(result));
}
