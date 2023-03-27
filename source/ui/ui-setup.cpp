
#include "ui-setup.hpp"
#include "version.hpp"

#include "warning-disable.hpp"
#include <QCoreApplication>
#include <QHBoxLayout>
#include <QPixmap>
#include <QSizePolicy>
#include <QVBoxLayout>
#include "warning-enable.hpp"

streamfx::ui::setup::setup(QWidget* parent) : QDialog(parent)
{
	// Don't delete this window when it is closed.

	// Set up basics, like not deleting self on close.
	setObjectName(QString::fromUtf8("setup"));
	setAttribute(Qt::WA_DeleteOnClose, false);
	setWindowFlag(Qt::WindowContextHelpButtonHint, false);
	setModal(false);

	// Set up overall layout.
	auto layout = new QVBoxLayout(this);
	layout->setContentsMargins(5, 5, 5, 5);
	layout->setSpacing(10);
	//setLayout(layout);

	// Create a font for multi-language support.
	QStringList families;
	families.append(QString::fromUtf8("Roboto"));
	families.append(QString::fromUtf8("MS Shell Dlg 2"));
	families.append(QString::fromUtf8("MS Gothic"));
	families.append(QString::fromUtf8("Malgun Gothic"));

	{ // Set up header.
		auto container_layout = new QHBoxLayout();
		container_layout->setContentsMargins(0, 0, 0, 0);
		container_layout->setSpacing(0);

		container_layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
		{
			auto header_layout = new QVBoxLayout();
			header_layout->setContentsMargins(0, 0, 0, 0);
			header_layout->setSpacing(10);

			{
				_header_logo = new QLabel(this);
				_header_logo->setObjectName(QString::fromUtf8("logo"));
				QSizePolicy size_policy(QSizePolicy::Preferred, QSizePolicy::Preferred);
				size_policy.setHorizontalStretch(0);
				size_policy.setVerticalStretch(0);
				size_policy.setHeightForWidth(_header_logo->sizePolicy().hasHeightForWidth());
				_header_logo->setSizePolicy(size_policy);
				{
					QPixmap logo{QString::fromUtf8(":/logos/streamfx_logo")};
					_header_logo->setPixmap(logo);
					_header_logo->setMinimumSize(logo.size());
					_header_logo->setMaximumSize(logo.size());
				}
				_header_logo->setScaledContents(false);
				header_layout->addWidget(_header_logo, 0, Qt::AlignCenter);
			}

			{
				auto links_layout = new QHBoxLayout();
				links_layout->setContentsMargins(0, 0, 0, 0);
				links_layout->setSpacing(10);
				header_layout->addLayout(links_layout);

				QFont font;
				font.setFamilies(families);
				font.setPointSize(12);

				QSizePolicy size_policy(QSizePolicy::Expanding, QSizePolicy::Maximum);
				size_policy.setHorizontalStretch(0);
				size_policy.setVerticalStretch(0);

				{
					_header_website = new QLabel(this);
					_header_website->setObjectName(QString::fromUtf8("website"));
					size_policy.setHeightForWidth(_header_website->sizePolicy().hasHeightForWidth());
					_header_website->setSizePolicy(size_policy);
					_header_website->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
					_header_website->setWordWrap(false);
					_header_website->setOpenExternalLinks(true);
					_header_website->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard
															 | Qt::LinksAccessibleByMouse);
					_header_website->setFont(font);
					links_layout->addWidget(_header_website);
				}

				{
					_header_patreon = new QLabel(this);
					_header_patreon->setObjectName(QString::fromUtf8("patreon"));
					size_policy.setHeightForWidth(_header_patreon->sizePolicy().hasHeightForWidth());
					_header_patreon->setSizePolicy(size_policy);
					_header_patreon->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
					_header_patreon->setWordWrap(false);
					_header_patreon->setOpenExternalLinks(true);
					_header_patreon->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard
															 | Qt::LinksAccessibleByMouse);
					_header_patreon->setFont(font);
					links_layout->addWidget(_header_patreon);
				}

				{
					_header_discord = new QLabel(this);
					_header_discord->setObjectName(QString::fromUtf8("discord"));
					size_policy.setHeightForWidth(_header_discord->sizePolicy().hasHeightForWidth());
					_header_discord->setSizePolicy(size_policy);
					_header_discord->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
					_header_discord->setWordWrap(false);
					_header_discord->setOpenExternalLinks(true);
					_header_discord->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard
															 | Qt::LinksAccessibleByMouse);
					_header_discord->setFont(font);
					links_layout->addWidget(_header_discord);
				}
			}

			container_layout->addLayout(header_layout);
		}
		container_layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));

		layout->addLayout(container_layout);
	}

	layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	{ // Set up footer.
		auto footer_layout = new QVBoxLayout();
		footer_layout->setContentsMargins(0, 0, 0, 0);
		footer_layout->setSpacing(10);

		{ // Banner
			auto container_layout = new QHBoxLayout();
			container_layout->setContentsMargins(0, 0, 0, 0);
			container_layout->setSpacing(0);

			{
				auto banner_layout = new QVBoxLayout();
				banner_layout->setContentsMargins(0, 0, 0, 0);
				banner_layout->setSpacing(10);

				{ // Title
					QFont font;
					font.setFamilies(families);
					font.setPointSize(9);

					_banner_title = new QLabel(this);
					_banner_title->setObjectName(QString::fromUtf8("banner_title"));
					_banner_title->setFont(font);
					banner_layout->addWidget(_banner_title, 0, Qt::AlignLeft | Qt::AlignVCenter);
				}

				{ // Content
					if (streamfx::ui::obs_browser_widget::is_available()) {
						_banner_content = new streamfx::ui::obs_browser_widget(
							QUrl(QString::fromUtf8("about:blank")), this);
					} else {
						_banner_content = new QLabel(this);
					}
					_banner_content->setObjectName(QString::fromUtf8("banner_content"));
					_banner_content->setFixedSize(728, 90);
					_banner_content->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
					banner_layout->addWidget(_banner_content, 0, Qt::AlignCenter);
				}

				container_layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
				container_layout->addLayout(banner_layout);
				container_layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
			}

			footer_layout->addLayout(container_layout);
		}

		{ // Controls
			auto control_layout = new QHBoxLayout();

			QFont font;
			font.setFamilies(families);
			font.setPointSize(12);

			{ // Cancel
				_control_cancel = new QPushButton(this);
				_control_cancel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
				_control_cancel->setFont(font);
				control_layout->addWidget(_control_cancel);
			}

			{ // Previous
				_control_prev = new QPushButton(this);
				_control_prev->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
				_control_prev->setFont(font);
				control_layout->addWidget(_control_prev);
			}

			control_layout->addSpacerItem(new QSpacerItem(100, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));

			{ // Next
				_control_next = new QPushButton(this);
				_control_next->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
				_control_next->setFont(font);
				control_layout->addWidget(_control_next);
			}

			{ // Finish
				_control_finish = new QPushButton(this);
				_control_finish->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
				_control_finish->setFont(font);
				control_layout->addWidget(_control_finish);
			}

			footer_layout->addLayout(control_layout);
		}

		{ // Info Line
			auto info_layout = new QHBoxLayout();

			QFont font;
			font.setFamilies(families);
			font.setPointSize(9);

			{ // Version
				_info_version = new QLabel(this);
				_info_version->setObjectName(QString::fromUtf8("version"));
				QSizePolicy size_policy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
				size_policy.setHorizontalStretch(0);
				size_policy.setVerticalStretch(0);
				size_policy.setHeightForWidth(_info_version->sizePolicy().hasHeightForWidth());
				_info_version->setSizePolicy(size_policy);
				_info_version->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
				_info_version->setWordWrap(false);
				_info_version->setOpenExternalLinks(true);
				_info_version->setTextInteractionFlags(Qt::LinksAccessibleByKeyboard | Qt::LinksAccessibleByMouse);
				_info_version->setFont(font);
				info_layout->addWidget(_info_version);
			}

			footer_layout->addLayout(info_layout);
		}

		layout->addLayout(footer_layout);
	}

	// Translate initial text.
	retranslate();

	// Connect any named objects directly.
	QMetaObject::connectSlotsByName(this);

	// Ensure we fit the content.
	resize(400, 300);
	adjustSize();
}

streamfx::ui::setup::~setup() {}

void streamfx::ui::setup::retranslate()
{
	setWindowTitle(QCoreApplication::translate("Setup", "StreamFX::UI.Setup.Title", ""));

	{ // Header

		{ // Website
			char buffer[2048];
			auto text = QCoreApplication::translate("Setup", "StreamFX::UI.Setup.Header.Website", "");
			snprintf(buffer, sizeof(buffer), "<a href='%s'>%s</a>", "https://streamfx.xaymar.com/",
					 text.toStdString().c_str());
			_header_website->setText(QString::fromUtf8(buffer));
		};

		{ // Patreon
			char buffer[2048];
			auto text = QCoreApplication::translate("Setup", "StreamFX::UI.Setup.Header.Patreon", "");
			snprintf(buffer, sizeof(buffer), "<a href='%s'>%s</a>", "https://s.xaymar.com/streamfx/patreon",
					 text.toStdString().c_str());
			_header_patreon->setText(QString::fromUtf8(buffer));
		};

		{ // Discord
			char buffer[2048];
			auto text = QCoreApplication::translate("Setup", "StreamFX::UI.Setup.Header.Discord", "");
			snprintf(buffer, sizeof(buffer), "<a href='%s'>%s</a>", "https://s.xaymar.com/streamfx/discord",
					 text.toStdString().c_str());
			_header_discord->setText(QString::fromUtf8(buffer));
		}
	}

	{ // Footer

		{ // Banner
			_banner_title->setText(QCoreApplication::translate("Setup", "StreamFX::UI.Setup.Banner"));
		}

		{ // Controls
			_control_cancel->setText(QCoreApplication::translate("Setup", "StreamFX::UI.Setup.Control.Cancel"));
			_control_prev->setText(QCoreApplication::translate("Setup", "StreamFX::UI.Setup.Control.Previous"));
			_control_next->setText(QCoreApplication::translate("Setup", "StreamFX::UI.Setup.Control.Next"));
			_control_finish->setText(QCoreApplication::translate("Setup", "StreamFX::UI.Setup.Control.Finish"));
		}

		_info_version->setText(STREAMFX_VERSION_STRING);
	}

	layout()->invalidate();
}
