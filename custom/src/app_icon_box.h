#pragma once

#include "base/object_ptr.h"

namespace Ui {
class RpWidget;
class VerticalLayout;
class GenericBox;
} // namespace Ui

namespace Window {
class SessionController;
} // namespace Window

void AppIconBox(not_null<Ui::GenericBox*> box);

[[nodiscard]] object_ptr<Ui::RpWidget> CreateAppIconButton(
	not_null<Ui::VerticalLayout*> container,
	not_null<Window::SessionController*> controller);
