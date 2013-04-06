/****************************************************************************
**
** Copyright (c) 2009-2013, Floware Computing (Pty) Ltd
**
** This file is part of Qtilities which is released under the following
** licensing options.
**
** Option 1: Open Source
** Under this license Qtilities is free software: you can
** redistribute it and/or modify it under the terms of the GNU General
** Public License as published by the Free Software Foundation, either
** version 3 of the License, or (at your option) any later version.
**
** Qtilities is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Qtilities. If not, see http://www.gnu.org/licenses/.
**
** Option 2: Commercial
** Alternatively, this library is also released under a commercial license
** that allows the development of closed source proprietary applications
** without restrictions on licensing. For more information on this option,
** please see the project website's licensing page:
** http://www.qtilities.org/licensing.html
**
** If you are unsure which license is appropriate for your use, please
** contact support@qtilities.org.
**
****************************************************************************/

#include "ProjectManagementPlugin.h"
#include "ProjectManagementPluginConstants.h"

#include <Qtilities.h>
#include <QtilitiesApplication.h>
#include <ProjectManager.h>
#include <ActionContainer.h>
#include <Command.h>
#include <QtilitiesCoreConstants.h>
#include <QtilitiesCoreGuiConstants.h>
#include <Logger.h>

#include <QtPlugin>
#include <QIcon>
#include <QLabel>
#include <QMessageBox>
#include <QFileDialog>

using namespace Qtilities::Core;
using namespace Qtilities::Core::Constants;
using namespace Qtilities::CoreGui;
using namespace Qtilities::CoreGui::Actions;
using namespace Qtilities::CoreGui::Constants;
using namespace Qtilities::ProjectManagement;
using namespace Qtilities::Plugins::ProjectManagement::Constants;

struct Qtilities::Plugins::ProjectManagement::ProjectManagementPluginPrivateData {
    ProjectManagementPluginPrivateData() : appended_project_name(QString()),
    is_initialized(false) {}

    QString         appended_project_name;
    bool            is_initialized;
};

Qtilities::Plugins::ProjectManagement::ProjectManagementPlugin::ProjectManagementPlugin(QObject* parent) : QObject(parent) {
    d = new ProjectManagementPluginPrivateData;
    setObjectName(pluginName());
    connect(PROJECT_MANAGER,SIGNAL(modificationStateChanged(bool)),SLOT(handle_projectStateChanged()));
    connect(PROJECT_MANAGER,SIGNAL(projectClosingFinished(bool)),SLOT(handle_projectStateChanged()));
    connect(PROJECT_MANAGER,SIGNAL(projectLoadingFinished(QString,bool)),SLOT(handle_projectStateChanged()));
    connect(PROJECT_MANAGER,SIGNAL(projectCreationFinished()),SLOT(handle_projectStateChanged()));
}

Qtilities::Plugins::ProjectManagement::ProjectManagementPlugin::~ProjectManagementPlugin() {
    delete d;
}

bool Qtilities::Plugins::ProjectManagement::ProjectManagementPlugin::initialize(const QStringList &arguments, QStringList *error_strings) {
    Q_UNUSED(arguments)
    Q_UNUSED(error_strings)

    // ---------------------------
    // Create and add project management actions:
    // Add project menu items
    // ---------------------------
    bool existed;
    ActionContainer* file_menu = ACTION_MANAGER->createMenu(PROJECT_MANAGER->projectMenuItemsTargetMenu(),existed);

    // ---------------------------
    // New Project
    // ---------------------------
    if (PROJECT_MANAGER->actionNewProject()) {
        Command* command = ACTION_MANAGER->command(qti_action_PROJECTS_NEW);
        file_menu->addAction(command,PROJECT_MANAGER->projectMenuItemsBeforeCommand());
    }
    // ---------------------------
    // Open Project
    // ---------------------------
    if (PROJECT_MANAGER->actionOpenProject()) {
        Command* command = ACTION_MANAGER->command(qti_action_PROJECTS_OPEN);
        file_menu->addAction(command,PROJECT_MANAGER->projectMenuItemsBeforeCommand());
    }
    // ---------------------------
    // Close Project
    // ---------------------------
    if (PROJECT_MANAGER->actionCloseProject()) {
        Command* command = ACTION_MANAGER->command(qti_action_PROJECTS_CLOSE);
        file_menu->addAction(command,PROJECT_MANAGER->projectMenuItemsBeforeCommand());
    }
    // ---------------------------
    // Save Project
    // ---------------------------
    if (PROJECT_MANAGER->actionSaveProject()) {
        Command* command = ACTION_MANAGER->command(qti_action_PROJECTS_SAVE);
        file_menu->addAction(command,PROJECT_MANAGER->projectMenuItemsBeforeCommand());
    }
    // ---------------------------
    // Save Project As
    // ---------------------------
    if (PROJECT_MANAGER->actionSaveAsProject()) {
        Command* command = ACTION_MANAGER->command(qti_action_PROJECTS_SAVE_AS);
        file_menu->addAction(command,PROJECT_MANAGER->projectMenuItemsBeforeCommand());
    }
    file_menu->addSeperator(PROJECT_MANAGER->projectMenuItemsBeforeCommand());
    // ---------------------------
    // Recent Projects Menu
    // ---------------------------
    ActionContainer* recent_projects_menu = PROJECT_MANAGER->recentProjectsMenuContainer();
    file_menu->addMenu(recent_projects_menu,PROJECT_MANAGER->projectMenuItemsBeforeCommand());

    // ---------------------------
    // Config Page
    // ---------------------------
    OBJECT_MANAGER->registerObject(PROJECT_MANAGER->configWidget(),QtilitiesCategory("GUI::Configuration Pages (IConfigPage)","::"));

    d->is_initialized = true;
    return true;
}

bool Qtilities::Plugins::ProjectManagement::ProjectManagementPlugin::initializeDependencies(QStringList *error_strings) {
    Q_UNUSED(error_strings)
    return true;
}

void Qtilities::Plugins::ProjectManagement::ProjectManagementPlugin::finalize() {

}

QString Qtilities::Plugins::ProjectManagement::ProjectManagementPlugin::pluginName() const {
    return "Project Management Plugin";
}

QtilitiesCategory Qtilities::Plugins::ProjectManagement::ProjectManagementPlugin::pluginCategory() const {
    return QtilitiesCategory(QApplication::applicationName());
}

Qtilities::Core::VersionInformation Qtilities::Plugins::ProjectManagement::ProjectManagementPlugin::pluginVersionInformation() const {
    VersionInformation version_info(qti_def_VERSION_MAJOR,qti_def_VERSION_MINOR,qti_def_VERSION_REVISION);
    return version_info;
}

QString Qtilities::Plugins::ProjectManagement::ProjectManagementPlugin::pluginPublisher() const {
    return "Floware Computing (Pty) Ltd";
}

QString Qtilities::Plugins::ProjectManagement::ProjectManagementPlugin::pluginPublisherWebsite() const {
    return "http://www.qtilities.org";
}

QString Qtilities::Plugins::ProjectManagement::ProjectManagementPlugin::pluginPublisherContact() const {
    return "support@qtilities.org";
}

QString Qtilities::Plugins::ProjectManagement::ProjectManagementPlugin::pluginDescription() const {
    return tr("A plugin which adds project management capabilities to the application.");
}

QString Qtilities::Plugins::ProjectManagement::ProjectManagementPlugin::pluginCopyright() const {
    return QString(tr("Copyright") + " 2009-2013, Floware Computing (Pty) Ltd");
}

QString Qtilities::Plugins::ProjectManagement::ProjectManagementPlugin::pluginLicense() const  {
    return tr("See the Qtilities Libraries license");
}

void Qtilities::Plugins::ProjectManagement::ProjectManagementPlugin::handle_projectStateChanged() {
    if (!d->is_initialized)
        return;

    IProject* project = PROJECT_MANAGER->currentProject();

    if (project) {
        // Check if the project name currenlty in the main window must change:
        QWidget* main_window = QtilitiesApplication::mainWindow();
        if (main_window && project) {
            // Name was never appended before.
            if (d->appended_project_name.isEmpty()) {
                QString new_title = main_window->windowTitle().trimmed();
                new_title.append(QString(" - %1").arg(project->projectName()));
                main_window->setWindowTitle(new_title);
                d->appended_project_name = project->projectName();
            } else {
                if (d->appended_project_name != project->projectName()) {
                    // The name of a previous project was appended.
                    QString new_title = main_window->windowTitle().trimmed();
                    new_title.chop(d->appended_project_name.length() + 3);
                    new_title.append(QString(" - %1").arg(project->projectName()));
                    main_window->setWindowTitle(new_title);
                    d->appended_project_name = project->projectName();
                } else {
                    // The name of the current project is already appended.
                }
            }
        }

        // Add the * character if neccesarry:
        if (project->isModified()) {
            QWidget* main_window = QtilitiesApplication::mainWindow();
            if (main_window) {
                if (!main_window->windowTitle().endsWith("*"))
                    main_window->setWindowTitle(main_window->windowTitle().append("*"));
            }
        } else {
            QWidget* main_window = QtilitiesApplication::mainWindow();
            if (main_window) {
                if (main_window->windowTitle().endsWith("*")) {
                    QString new_title = main_window->windowTitle();
                    new_title.chop(1);
                    main_window->setWindowTitle(new_title);
                }
            }
        }
    } else {
        QWidget* main_window = QtilitiesApplication::mainWindow();
        if (main_window) {
            // Remove possible *:
            if (main_window->windowTitle().endsWith("*")) {
                QString new_title = main_window->windowTitle();
                new_title.chop(1);
                main_window->setWindowTitle(new_title);
            }

            // Remove possible last appended name:
            if (!d->appended_project_name.isEmpty()) {
                QString new_title = main_window->windowTitle();
                new_title.chop(d->appended_project_name.length() + 3);
                main_window->setWindowTitle(new_title);
                d->appended_project_name.clear();
            }
        }
    }
}

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    using namespace Qtilities::Plugins::ProjectManagement;
    Q_EXPORT_PLUGIN2(ProjectManagementPlugin, ProjectManagementPlugin)
#endif
