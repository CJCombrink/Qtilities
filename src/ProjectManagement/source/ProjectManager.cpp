/****************************************************************************
**
** Copyright (c) 2009-2010, Jaco Naude
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

#include "ProjectManager.h"
#include "Project.h"
#include "ProjectManagementConfig.h"

#include <Logger.h>

#include <QMutex>
#include <QCoreApplication>
#include <QMap>
#include <QPointer>
#include <QSettings>
#include <QApplication>

#include <QMessageBox>
#include <QFileDialog>

using namespace Qtilities::Core;
using namespace Qtilities::ProjectManagement::Constants;
using namespace Qtilities::Logging;

struct Qtilities::ProjectManagement::ProjectManagerData  {
    ProjectManagerData() : current_project(0),
    recent_projects_size(5),
    config_widget(0),
    project_file_version(0) {}

    QPointer<Project>               current_project;
    QList<IProjectItem*>            item_list;
    QMap<QString, QVariant>         recent_project_names;
    QStringList                  recent_project_stack;
    int                             recent_projects_size;
    ProjectManagementConfig*        config_widget;
    bool                            open_last_project;
    bool                            auto_create_new_project;
    bool                            use_custom_projects_path;
    QString                         custom_projects_path;
    bool                            check_modified_projects;
    quint32                         project_file_version;
    ProjectManager::ModifiedProjectsHandlingPolicy  modified_projects_handling_policy;
};

Qtilities::ProjectManagement::ProjectManager* Qtilities::ProjectManagement::ProjectManager::m_Instance = 0;

Qtilities::ProjectManagement::ProjectManager* Qtilities::ProjectManagement::ProjectManager::instance()
{
    static QMutex mutex;
    if (!m_Instance)
    {
      mutex.lock();

      if (!m_Instance)
        m_Instance = new ProjectManager;

      mutex.unlock();
    }

    return m_Instance;
}

Qtilities::ProjectManagement::ProjectManager::ProjectManager(QObject* parent) : QObject(parent)
{    
    d = new ProjectManagerData;
    readSettings();
}

Qtilities::ProjectManagement::ProjectManager::~ProjectManager()
{
    delete d;
}

bool Qtilities::ProjectManagement::ProjectManager::newProject() {
    // Check if a current project exist.
    // If not, create it.
    if (!d->current_project) {
        d->current_project = new Project();
        connect(d->current_project,SIGNAL(modificationStateChanged(bool)),SLOT(setModificationState(bool)));
        d->current_project->setProjectItems(d->item_list,true);
        return true;
    } else {
        closeProject();
        d->current_project = new Project();
        connect(d->current_project,SIGNAL(modificationStateChanged(bool)),SLOT(setModificationState(bool)));
        d->current_project->setProjectItems(d->item_list,true);
        return true;
    }

    return false;
}

bool Qtilities::ProjectManagement::ProjectManager::closeProject(){
    if (d->current_project) {
        if (d->current_project->isModified()) {
            saveProject(d->current_project->projectFile());
        }

        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        d->current_project->closeProject();
        QApplication::restoreOverrideCursor();
        d->current_project->disconnect(this);
        delete d->current_project;
        d->current_project = 0;
        emit modificationStateChanged(false);
    }
    return true;
}

bool Qtilities::ProjectManagement::ProjectManager::openProject(const QString& file_name) {
    // If the project is already open tell the user, don't open it again.
    if (d->current_project) {
        if (d->current_project->projectFile() == file_name) {
            LOG_INFO(QString(tr("The project you are trying to open is already open.")));
            return false;
        }
        if (!closeProject())
            return false;
        delete d->current_project;
    }

    d->current_project = new Project(this);
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    connect(d->current_project,SIGNAL(modificationStateChanged(bool)),SLOT(setModificationState(bool)));
    d->current_project->setProjectItems(d->item_list);
    if (!d->current_project->loadProject(file_name,false)) {
        closeProject();
        QApplication::restoreOverrideCursor();
        return false;
    }

    addRecentProject(d->current_project);
    // Still not sure why we need to call it again here:
    IModificationNotifier::NotificationTargets notify_targets = 0;
    notify_targets |= IModificationNotifier::NotifyListeners;
    notify_targets |= IModificationNotifier::NotifySubjects;
    setModificationState(false,notify_targets);
    QApplication::restoreOverrideCursor();
    return true;
}

bool Qtilities::ProjectManagement::ProjectManager::saveProject(QString file_name) {
    if (!d->current_project)
        return false;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    if (file_name.isEmpty()) {
        if (d->current_project->projectFile().isEmpty()) {
            QString filter = QString(tr("Project File (*%1)")).arg(FILE_EXT_PROJECT);
            QString project_path = QCoreApplication::applicationDirPath() + "/Projects";
            QString file_name = QFileDialog::getSaveFileName(0, tr("Save Project"),project_path, filter);
            if (file_name.isEmpty())
                return false;
            else {
                if (!file_name.endsWith(FILE_EXT_PROJECT))
                    file_name.append(FILE_EXT_PROJECT);
            }
        } else
            file_name = d->current_project->projectFile();
    }

    if (d->current_project->saveProject(file_name)) {
        addRecentProject(d->current_project);
        QApplication::restoreOverrideCursor();
        return true;
    }

    QApplication::restoreOverrideCursor();
    return false;
}

QString Qtilities::ProjectManagement::ProjectManager::currentProjectName() const {
    if (d->current_project)
        return d->current_project->projectName();
    else
        return QString();
}

QString Qtilities::ProjectManagement::ProjectManager::currentProjectFileName() const {
    if (d->current_project)
        return d->current_project->projectFile();
    else
        return QString();
}

Qtilities::ProjectManagement::IProject* Qtilities::ProjectManagement::ProjectManager::currentProject() const {
    return d->current_project;
}

QStringList Qtilities::ProjectManagement::ProjectManager::recentProjectNames() {
    QStringList recent_project_names;
    for (int i = 0; i < d->recent_project_names.values().count();i++)
        recent_project_names << d->recent_project_names.values().at(i).toString();
    return recent_project_names;
}

QStringList Qtilities::ProjectManagement::ProjectManager::recentProjectPaths() {
    return d->recent_project_names.keys();
}

QString Qtilities::ProjectManagement::ProjectManager::recentProjectPath(const QString& recent_project_name) {
    for (int i = 0; i < d->recent_project_names.size(); i++) {
        if (d->recent_project_names.values().at(i).toString() == recent_project_name)
            return d->recent_project_names.keys().at(i);
    }
    return QString();
}

void Qtilities::ProjectManagement::ProjectManager::clearRecentProjects() {
    d->recent_project_names.clear();
    d->recent_project_stack.clear();
    LOG_INFO(tr("Successfully cleared recent project list."));
}

void Qtilities::ProjectManagement::ProjectManager::setProjectItemList(QList<IProjectItem*> item_list) {
    d->item_list = item_list;
}

void Qtilities::ProjectManagement::ProjectManager::setProjectFileVersion(quint32 project_file_version) {
    d->project_file_version = project_file_version;
}

quint32 Qtilities::ProjectManagement::ProjectManager::projectFileVersion() const {
    return d->project_file_version;
}

QWidget* Qtilities::ProjectManagement::ProjectManager::configWidget() {
    if (!d->config_widget)
        d->config_widget = new ProjectManagementConfig();

    return d->config_widget;
}

void Qtilities::ProjectManagement::ProjectManager::setOpenLastProjectOnStartup(bool toggle) {
    d->open_last_project = toggle;
    writeSettings();
}

bool Qtilities::ProjectManagement::ProjectManager::openLastProjectOnStartup() const {
    return d->open_last_project;
}

void Qtilities::ProjectManagement::ProjectManager::setCreateNewProjectOnStartup(bool toggle) {
    d->auto_create_new_project = toggle;
    writeSettings();
}

bool Qtilities::ProjectManagement::ProjectManager::createNewProjectOnStartup() const {
    return d->auto_create_new_project;
}

void Qtilities::ProjectManagement::ProjectManager::setCustomProjectsPath(const QString& projects_path) {
    d->custom_projects_path = projects_path;
}

QString Qtilities::ProjectManagement::ProjectManager::customProjectsPath() const {
    return d->custom_projects_path;
}

bool Qtilities::ProjectManagement::ProjectManager::useCustomProjectsPath() const {
    return d->use_custom_projects_path;
}

void Qtilities::ProjectManagement::ProjectManager::setUseCustomProjectsPath(bool toggle) {
    d->use_custom_projects_path = toggle;
}

bool Qtilities::ProjectManagement::ProjectManager::checkModifiedOpenProjects() const {
    return d->check_modified_projects;
}

void Qtilities::ProjectManagement::ProjectManager::setCheckModifiedOpenProjects(bool toggle) {
    d->check_modified_projects = toggle;
}

Qtilities::ProjectManagement::ProjectManager::ModifiedProjectsHandlingPolicy Qtilities::ProjectManagement::ProjectManager::modifiedProjectsHandlingPolicy() const {
    return d->modified_projects_handling_policy;
}

void Qtilities::ProjectManagement::ProjectManager::setModifiedProjectsHandlingPolicy(ModifiedProjectsHandlingPolicy handling_policy) {
    d->modified_projects_handling_policy = handling_policy;

}

void Qtilities::ProjectManagement::ProjectManager::writeSettings() const {
    // Store settings using QSettings only if it was initialized
    QSettings settings;
    settings.beginGroup("Projects");
    settings.setValue("open_last_project", d->open_last_project);
    settings.setValue("auto_create_new_project", d->auto_create_new_project);
    settings.setValue("recent_project_map", QVariant(d->recent_project_names));
    settings.setValue("recent_project_stack", QVariant(d->recent_project_stack));
    settings.setValue("use_custom_projects_path", d->use_custom_projects_path);
    settings.setValue("check_modified_projects", d->check_modified_projects);
    settings.setValue("modified_projects_handling_policy", QVariant((int) d->modified_projects_handling_policy));
    settings.setValue("custom_projects_path", QVariant(d->custom_projects_path));
    settings.endGroup();
}

void Qtilities::ProjectManagement::ProjectManager::readSettings() {
    if (QCoreApplication::organizationName().isEmpty() || QCoreApplication::organizationDomain().isEmpty() || QCoreApplication::applicationName().isEmpty())
        LOG_DEBUG("The project manager may not be able to restore paramaters from previous sessions since the correct details in QCoreApplication have not been set.");

    // Load project management paramaters using QSettings()
    QSettings settings;
    settings.beginGroup("Projects");
    d->open_last_project = settings.value("open_last_project", false).toBool();
    d->auto_create_new_project = settings.value("auto_create_new_project", false).toBool();
    d->use_custom_projects_path = settings.value("use_custom_projects_path", false).toBool();
    d->check_modified_projects = settings.value("check_modified_projects", true).toBool();
    d->modified_projects_handling_policy = (ProjectManager::ModifiedProjectsHandlingPolicy) settings.value("modified_projects_handling_policy", 0).toInt();
    d->custom_projects_path = settings.value("custom_projects_path",QCoreApplication::applicationDirPath() + "/projects").toString();
    d->recent_project_names = settings.value("recent_project_map", false).toMap();
    QList<QVariant> variant_stack_list = settings.value("recent_project_stack", false).toList();
    QStringList string_stack_list;
    for (int i = 0; i < variant_stack_list.count(); i++)
        string_stack_list << variant_stack_list.at(i).toString();
    d->recent_project_stack = string_stack_list;
    settings.endGroup();
}

void Qtilities::ProjectManagement::ProjectManager::initialize() {
    LOG_INFO(tr("Qtilities Project Management Framework, initialization started..."));

    // Settings was already read in the constructor.
    if (d->open_last_project) {
        QString last_project;
        if (d->recent_project_names.count() > 0)
            last_project = d->recent_project_stack.front();
        if (!last_project.isEmpty()) {
            LOG_INFO(tr("Loading project from last session from path: ") + last_project);
            if (!openProject(last_project)) {
                // We create a empty project when the last project was not valid and auto create project is set.
                if (newProject())
                    LOG_INFO(tr("Successfully created new project in place of failed project."));
                else
                    LOG_ERROR(tr("Failed to create a new project in place of failed project."));
            }
        } else if (d->auto_create_new_project) {
            if (newProject())
                LOG_INFO(tr("Successfully created new project on application startup."));
            else
                LOG_ERROR(tr("Failed to create a new project on startup."));
        }
    }

    LOG_INFO(tr("Qtilities Project Management Framework, initialization finished successfully..."));
}

void Qtilities::ProjectManagement::ProjectManager::finalize() {
    if (d->current_project) {
        if (d->check_modified_projects && d->current_project->isModified()) {
            if (d->modified_projects_handling_policy == PromptUser) {
                QMessageBox msgBox;
                msgBox.setWindowTitle("Project Manager");
                msgBox.setText("Do you want to save your current project before you exit?");
                msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
                msgBox.setDefaultButton(QMessageBox::Yes);
                int ret = msgBox.exec();
                if (ret == QMessageBox::Yes) {
                    saveProject();
                }
            } else if (d->modified_projects_handling_policy == AutoSave) {
                saveProject();
            }
        }
    }

    writeSettings();
}

void Qtilities::ProjectManagement::ProjectManager::addRecentProject(IProject* project) {
    if (!project)
        return;

    // First check if this project already exists in the list.
    // If so, move it to the first position and exit.
    if (d->recent_project_stack.contains(project->projectFile())) {
        // Remove current.
        d->recent_project_stack.removeOne(project->projectFile());
        // Add again in new position.
        d->recent_project_stack.push_front(project->projectFile());
        return;
    }

    // Check the required size of the recent projects list.
    if (d->recent_project_stack.count() == d->recent_projects_size) {
        d->recent_project_names.remove(d->recent_project_stack.last());
        d->recent_project_stack.removeLast();
    }

    // Now add the new project.
    d->recent_project_names[project->projectFile()] = project->projectName();
    d->recent_project_stack.push_front(project->projectFile());
}

bool Qtilities::ProjectManagement::ProjectManager::isModified() const {
    if (d->current_project) {
        return d->current_project->isModified();
    }

    return false;
}

void Qtilities::ProjectManagement::ProjectManager::setModificationState(bool new_state, IModificationNotifier::NotificationTargets notification_targets) {
    if (notification_targets & IModificationNotifier::NotifyListeners) {
        emit modificationStateChanged(new_state);
    }
    if (notification_targets & IModificationNotifier::NotifySubjects) {
        d->current_project->setModificationState(new_state,notification_targets);
    }
}

