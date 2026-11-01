#pragma once

#include "base/object_ptr.h"
#include "settings/settings_type.h"

namespace Ui {
class RpWidget;
class VerticalLayout;
} // namespace Ui

namespace Settings {

[[nodiscard]] Type AppIconId();

} // namespace Settings

[[nodiscard]] object_ptr<Ui::RpWidget> CreateAppIconButton(
	not_null<Ui::VerticalLayout*> container,
	Fn<void(Settings::Type)> showOther);
