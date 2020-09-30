// Copyright (c) 2020 Michael Fabian Dirks <info@xaymar.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "ui-updater.hpp"
#include "common.hpp"

#define ST_PREFIX "<ui::updater> "
#define D_LOG_ERROR(...) DLOG_ERROR(ST_PREFIX __VA_ARGS__)
#define D_LOG_WARNING(...) DLOG_WARNING(ST_PREFIX __VA_ARGS__)
#define D_LOG_INFO(...) DLOG_INFO(ST_PREFIX __VA_ARGS__)
#ifdef _DEBUG
#define D_LOG_DEBUG(...) DLOG_DEBUG(ST_PREFIX __VA_ARGS__)
#else
#define D_LOG_DEBUG(...)
#endif

#define D_I18N_MENU_CHECKFORUPDATES "UI.Updater.Menu.CheckForUpdates"
#define D_I18N_MENU_CHECKFORUPDATES_AUTOMATICALLY "UI.Updater.Menu.CheckForUpdates.Automatically"
#define D_I18N_MENU_CHANNEL "UI.Updater.Menu.Channel"
#define D_I18N_MENU_CHANNEL_RELEASE "UI.Updater.Menu.Channel.Release"
#define D_I18N_MENU_CHANNEL_TESTING "UI.Updater.Menu.Channel.Testing"
#define D_I18N_DIALOG_TITLE "UI.Updater.Dialog.Title"
#define D_I18N_GITHUBPERMISSION_TITLE "UI.Updater.GitHubPermission.Title"
#define D_I18N_GITHUBPERMISSION_TEXT "UI.Updater.GitHubPermission.Text"

streamfx::ui::updater_dialog::updater_dialog() : QDialog(reinterpret_cast<QWidget*>(obs_frontend_get_main_window()))
{
	setupUi(this);
	setWindowFlag(Qt::WindowContextHelpButtonHint, false);
	setWindowFlag(Qt::WindowMinimizeButtonHint, false);
	setWindowFlag(Qt::WindowMaximizeButtonHint, false);

	connect(ok, &QPushButton::clicked, this, &streamfx::ui::updater_dialog::on_ok);
	connect(cancel, &QPushButton::clicked, this, &streamfx::ui::updater_dialog::on_cancel);
}

streamfx::ui::updater_dialog::~updater_dialog() {}

void streamfx::ui::updater_dialog::show(streamfx::update_info current, streamfx::update_info update)
{
	{
		std::vector<char> buf;
		if (current.version_type) {
			buf.resize(static_cast<size_t>(snprintf(nullptr, 0, "%" PRIu16 ".%" PRIu16 ".%" PRIu16 "%.1s%" PRIu16,
													current.version_major, current.version_minor, current.version_patch,
													&current.version_type, current.version_index))
					   + 1);
			snprintf(buf.data(), buf.size(), "%" PRIu16 ".%" PRIu16 ".%" PRIu16 "%.1s%" PRIu16, current.version_major,
					 current.version_minor, current.version_patch, &current.version_type, current.version_index);
		} else {
			buf.resize(
				static_cast<size_t>(snprintf(nullptr, 0, "%" PRIu16 ".%" PRIu16 ".%" PRIu16, current.version_major,
											 current.version_minor, current.version_patch))
				+ 1);
			snprintf(buf.data(), buf.size(), "%" PRIu16 ".%" PRIu16 ".%" PRIu16, current.version_major,
					 current.version_minor, current.version_patch);
		}
		currentVersion->setText(QString::fromUtf8(buf.data()));
	}

	{
		std::vector<char> buf;
		if (update.version_type) {
			buf.resize(static_cast<size_t>(snprintf(nullptr, 0, "%" PRIu16 ".%" PRIu16 ".%" PRIu16 "%.1s%" PRIu16,
													update.version_major, update.version_minor, update.version_patch,
													&update.version_type, update.version_index))
					   + 1);
			snprintf(buf.data(), buf.size(), "%" PRIu16 ".%" PRIu16 ".%" PRIu16 "%.1s%" PRIu16, update.version_major,
					 update.version_minor, update.version_patch, &update.version_type, update.version_index);
		} else {
			buf.resize(static_cast<size_t>(snprintf(nullptr, 0, "%" PRIu16 ".%" PRIu16 ".%" PRIu16,
													update.version_major, update.version_minor, update.version_patch))
					   + 1);
			snprintf(buf.data(), buf.size(), "%" PRIu16 ".%" PRIu16 ".%" PRIu16, update.version_major,
					 update.version_minor, update.version_patch);
		}
		latestVersion->setText(QString::fromUtf8(buf.data()));

		{
			std::vector<char> buf2;
			buf2.resize(static_cast<size_t>(snprintf(nullptr, 0, D_TRANSLATE(D_I18N_DIALOG_TITLE), buf.data())) + 1);
			snprintf(buf2.data(), buf2.size(), D_TRANSLATE(D_I18N_DIALOG_TITLE), buf.data());
			setWindowTitle(QString::fromUtf8(buf2.data()));
		}
	}

	_update_url = QUrl(QString::fromStdString(update.url));

	this->setModal(true);
	QDialog::show();
}

void streamfx::ui::updater_dialog::hide()
{
	this->setModal(false);
	QDialog::hide();
}

void streamfx::ui::updater_dialog::on_ok()
{
	QDesktopServices::openUrl(_update_url);
	hide();
}

void streamfx::ui::updater_dialog::on_cancel()
{
	hide();
}

streamfx::ui::updater::updater(QMenu* menu)
{
	// Create dialog.
	_dialog = new updater_dialog();

	{ // Create the necessary menu entries.
		menu->addSeparator();

		// Check for Updates
		_cfu = menu->addAction(QString::fromUtf8(D_TRANSLATE(D_I18N_MENU_CHECKFORUPDATES)));
		connect(_cfu, &QAction::triggered, this, &streamfx::ui::updater::on_cfu_triggered);

		// Automatically check for Updates
		_cfu_auto = menu->addAction(QString::fromUtf8(D_TRANSLATE(D_I18N_MENU_CHECKFORUPDATES_AUTOMATICALLY)));
		_cfu_auto->setCheckable(true);
		connect(_cfu_auto, &QAction::toggled, this, &streamfx::ui::updater::on_cfu_auto_toggled);

		// Update Channel
		_channel_menu = menu->addMenu(QString::fromUtf8(D_TRANSLATE(D_I18N_MENU_CHANNEL)));

		_channel_stable = _channel_menu->addAction(QString::fromUtf8(D_TRANSLATE(D_I18N_MENU_CHANNEL_RELEASE)));
		_channel_stable->setCheckable(true);

		_channel_preview = _channel_menu->addAction(QString::fromUtf8(D_TRANSLATE(D_I18N_MENU_CHANNEL_TESTING)));
		_channel_preview->setCheckable(true);

		_channel_group = new QActionGroup(_channel_menu);
		_channel_group->addAction(_channel_stable);
		_channel_group->addAction(_channel_preview);
		connect(_channel_group, &QActionGroup::triggered, this, &streamfx::ui::updater::on_channel_group_triggered);
	}

	// Connect internal signals.
	connect(this, &streamfx::ui::updater::autoupdate_changed, this, &streamfx::ui::updater::on_autoupdate_changed,
			Qt::QueuedConnection);
	connect(this, &streamfx::ui::updater::channel_changed, this, &streamfx::ui::updater::on_channel_changed,
			Qt::QueuedConnection);
	connect(this, &streamfx::ui::updater::update_detected, this, &streamfx::ui::updater::on_update_detected,
			Qt::QueuedConnection);
	connect(this, &streamfx::ui::updater::check_active, this, &streamfx::ui::updater::on_check_active,
			Qt::QueuedConnection);

	{ // Retrieve the updater object and listen to it.
		_updater = streamfx::updater::instance();
		_updater->events.automation_changed.add(std::bind(&streamfx::ui::updater::on_updater_automation_changed, this,
														  std::placeholders::_1, std::placeholders::_2));
		_updater->events.channel_changed.add(std::bind(&streamfx::ui::updater::on_updater_channel_changed, this,
													   std::placeholders::_1, std::placeholders::_2));
		_updater->events.refreshed.add(
			std::bind(&streamfx::ui::updater::on_updater_refreshed, this, std::placeholders::_1));
		if (_updater->automation()) {
			if (_updater->gdpr()) {
				_updater->refresh();
			} else {
				create_gdpr_box();
				_gdpr->exec();
			}
		}

		// Sync with updater information.
		emit autoupdate_changed(_updater->automation());
		emit channel_changed(_updater->channel());
	}
}

streamfx::ui::updater::~updater() {}

void streamfx::ui::updater::on_updater_automation_changed(streamfx::updater&, bool value)
{
	emit autoupdate_changed(value);
}

void streamfx::ui::updater::on_updater_channel_changed(streamfx::updater&, streamfx::update_channel channel)
{
	emit channel_changed(channel);
}

void streamfx::ui::updater::on_updater_refreshed(streamfx::updater&)
{
	emit check_active(false);

	if (!_updater->have_update())
		return;

	emit update_detected();
}

void streamfx::ui::updater::on_channel_changed(streamfx::update_channel channel)
{
	bool is_stable = channel == streamfx::update_channel::RELEASE;
	_channel_stable->setChecked(is_stable);
	_channel_preview->setChecked(!is_stable);
}

void streamfx::ui::updater::on_update_detected()
{
	_dialog->show(_updater->get_current_info(), _updater->get_update_info());
}

void streamfx::ui::updater::on_autoupdate_changed(bool enabled)
{
	_cfu_auto->setChecked(enabled);
}

void streamfx::ui::updater::on_gdpr_button(QAbstractButton* btn)
{
	if (_gdpr->standardButton(btn) == QMessageBox::Ok) {
		_updater->set_gdpr(true);
		emit check_active(true);
		_updater->refresh();
	} else {
		_updater->set_gdpr(false);
		_updater->set_automation(false);
	}
}

void streamfx::ui::updater::on_cfu_triggered(bool)
{
	if (!_updater->gdpr()) {
		create_gdpr_box();
		_gdpr->exec();
	} else {
		emit check_active(true);
		_updater->refresh();
	}
}

void streamfx::ui::updater::on_cfu_auto_toggled(bool flag)
{
	_updater->set_automation(flag);
}

void streamfx::ui::updater::on_channel_group_triggered(QAction* action)
{
	if (action == _channel_stable) {
		_updater->set_channel(update_channel::RELEASE);
	} else {
		_updater->set_channel(update_channel::TESTING);
	}
}

std::shared_ptr<streamfx::ui::updater> streamfx::ui::updater::instance(QMenu* menu)
{
	static std::weak_ptr<streamfx::ui::updater> _instance;
	static std::mutex                           _lock;

	auto lock = std::lock_guard<std::mutex>(_lock);
	if (_instance.expired() && menu) {
		auto ptr  = std::make_shared<streamfx::ui::updater>(menu);
		_instance = ptr;
		return ptr;
	} else {
		return _instance.lock();
	}
}

void streamfx::ui::updater::on_check_active(bool active)
{
	_cfu->setEnabled(!active);
	_channel_group->setEnabled(!active);
	_channel_preview->setEnabled(!active);
	_channel_stable->setEnabled(!active);
	_channel_menu->setEnabled(!active);
}

void streamfx::ui::updater::create_gdpr_box()
{
	if (_gdpr) {
		_gdpr->deleteLater();
		_gdpr = nullptr;
	}

	// Create GitHub message box.
	_gdpr = new QMessageBox(reinterpret_cast<QWidget*>(obs_frontend_get_main_window()));
	_gdpr->setWindowTitle(QString::fromUtf8(D_TRANSLATE(D_I18N_GITHUBPERMISSION_TITLE)));
	_gdpr->setTextFormat(Qt::TextFormat::RichText);
	_gdpr->setText(QString::fromUtf8(D_TRANSLATE(D_I18N_GITHUBPERMISSION_TEXT)));
	_gdpr->setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
	connect(_gdpr, &QMessageBox::buttonClicked, this, &streamfx::ui::updater::on_gdpr_button);
}
