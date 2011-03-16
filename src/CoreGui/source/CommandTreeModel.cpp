/****************************************************************************
**
** Copyright (c) 2009-2011, Jaco Naude
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

#include "CommandTreeModel.h"
#include "QtilitiesCoreGuiConstants.h"
#include "QtilitiesApplication.h"
#include "Command.h"

#include <QtGui>

Qtilities::CoreGui::CommandTreeModel::CommandTreeModel(QObject* parent) : ObserverTreeModel(parent) {

}

QVariant Qtilities::CoreGui::CommandTreeModel::data(const QModelIndex &index, int role) const {
    if ((index.column() == columnCount() - 2) && (role == Qt::DisplayRole)) {
        QObject* obj = getObject(index);
        if (obj) {
            Command* command = qobject_cast<Command*> (obj);
            if (command) {
                return command->text();
            }
        }
        return QVariant();
    } else if ((index.column() == columnCount() - 1) && (role == Qt::DisplayRole || role == Qt::EditRole)) {
        QObject* obj = getObject(index);
        if (obj) {
            Command* command = qobject_cast<Command*> (obj);
            if (command) {
                return command->keySequence().toString();
            }
        }
        return QVariant();
    } else if ((index.column() == columnCount() - 1) && (role == Qt::ForegroundRole)) {
        QObject* obj = getObject(index);
        if (obj) {
            Command* command = qobject_cast<Command*> (obj);
            if (command) {
                if (ACTION_MANAGER->commandsWithKeySequence(command->keySequence()).count() > 1)
                    return QBrush(Qt::red);
            }
        }
        return QVariant();
    } else {
        return ObserverTreeModel::data(index,role);
    }
}

Qt::ItemFlags Qtilities::CoreGui::CommandTreeModel::flags(const QModelIndex &index) const {
    if (index.column() == columnCount() - 2) {
        Qt::ItemFlags item_flags = 0;
        item_flags |= Qt::ItemIsEnabled;
        item_flags |= Qt::ItemIsSelectable;
        return item_flags;
    } else if (index.column() == columnCount() - 1) {
        Qt::ItemFlags item_flags = 0;
        item_flags |= Qt::ItemIsEnabled;
        item_flags |= Qt::ItemIsSelectable;
        item_flags |= Qt::ItemIsEditable;
        return item_flags;
    } else {
        return ObserverTreeModel::flags(index);
    }
}

QVariant Qtilities::CoreGui::CommandTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if ((section == columnCount() - 2) && (role == Qt::DisplayRole)) {
        return QString("Label");
    } else if ((section == columnCount() - 1) && (role == Qt::DisplayRole)) {
        return QString("Shortcut");
    } else {
        return ObserverTreeModel::headerData(section,orientation,role);
    }
}

bool Qtilities::CoreGui::CommandTreeModel::setData(const QModelIndex &index, const QVariant &value, int role) {
    if (index.column() == columnCount() - 2) {
        return false;
    } else if (index.column() == columnCount() - 1 && role == Qt::EditRole) {
        QObject* obj = getObject(index);
        if (obj) {
            Command* command = qobject_cast<Command*> (obj);
            if (command) {
                command->setKeySequence(QKeySequence(value.toString()));
                return true;
            }
        }
        return false;
    } else {
        return ObserverTreeModel::setData(index,value,role);
    }
}

int Qtilities::CoreGui::CommandTreeModel::rowCount(const QModelIndex &parent) const {
    return ObserverTreeModel::rowCount(parent);
}

int Qtilities::CoreGui::CommandTreeModel::columnCount(const QModelIndex &parent) const {
    return ObserverTreeModel::columnCount(parent) + 2;
}

// -----------------------------------------------
// ShortcutEditorDelegate
// -----------------------------------------------

Qtilities::CoreGui::ShortcutEditorDelegate::ShortcutEditorDelegate(QObject *parent) : QItemDelegate(parent) {

}

QWidget *Qtilities::CoreGui::ShortcutEditorDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    Q_UNUSED(option)
    Q_UNUSED(index)

    QtKeySequenceEdit *editor = new QtKeySequenceEdit(parent);
    return editor;
}

void Qtilities::CoreGui::ShortcutEditorDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
    QString value = index.model()->data(index, Qt::DisplayRole).toString();
    QtKeySequenceEdit *shortcut_editor = static_cast<QtKeySequenceEdit*>(editor);
    shortcut_editor->setKeySequence(QKeySequence(value));
}

void Qtilities::CoreGui::ShortcutEditorDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
    QtKeySequenceEdit *shortcut_editor = static_cast<QtKeySequenceEdit*>(editor);
    QString value = shortcut_editor->keySequence().toString();
    model->setData(index, value, Qt::EditRole);
}

void Qtilities::CoreGui::ShortcutEditorDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    Q_UNUSED(index)
    editor->setGeometry(option.rect);
}

// -----------------------------------------------
// QtKeySequenceEdit
// -----------------------------------------------
// This class is unmodified from the Qt Property
// Browser solution.
// -----------------------------------------------

Qtilities::CoreGui::QtKeySequenceEdit::QtKeySequenceEdit(QWidget *parent)
    : QWidget(parent), m_num(0), m_lineEdit(new QLineEdit(this))
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(m_lineEdit);
    layout->setMargin(0);
    m_lineEdit->installEventFilter(this);
    m_lineEdit->setReadOnly(true);
    m_lineEdit->setFocusProxy(this);
    setFocusPolicy(m_lineEdit->focusPolicy());
    setAttribute(Qt::WA_InputMethodEnabled);
}

bool Qtilities::CoreGui::QtKeySequenceEdit::eventFilter(QObject *o, QEvent *e)
{
    if (o == m_lineEdit && e->type() == QEvent::ContextMenu) {
        QContextMenuEvent *c = static_cast<QContextMenuEvent *>(e);
        QMenu *menu = m_lineEdit->createStandardContextMenu();
        const QList<QAction *> actions = menu->actions();
        QListIterator<QAction *> itAction(actions);
        while (itAction.hasNext()) {
            QAction *action = itAction.next();
            action->setShortcut(QKeySequence());
            QString actionString = action->text();
            const int pos = actionString.lastIndexOf(QLatin1Char('\t'));
            if (pos > 0)
                actionString.remove(pos, actionString.length() - pos);
            action->setText(actionString);
        }
        QAction *actionBefore = 0;
        if (actions.count() > 0)
            actionBefore = actions[0];
        QAction *clearAction = new QAction(tr("Clear Shortcut"), menu);
        menu->insertAction(actionBefore, clearAction);
        menu->insertSeparator(actionBefore);
        clearAction->setEnabled(!m_keySequence.isEmpty());
        connect(clearAction, SIGNAL(triggered()), this, SLOT(slotClearShortcut()));
        menu->exec(c->globalPos());
        delete menu;
        e->accept();
        return true;
    }

    return QWidget::eventFilter(o, e);
}

void Qtilities::CoreGui::QtKeySequenceEdit::slotClearShortcut()
{
    if (m_keySequence.isEmpty())
        return;
    setKeySequence(QKeySequence());
    emit keySequenceChanged(m_keySequence);
}

void Qtilities::CoreGui::QtKeySequenceEdit::handleKeyEvent(QKeyEvent *e)
{
    int nextKey = e->key();
    if (nextKey == Qt::Key_Control || nextKey == Qt::Key_Shift ||
            nextKey == Qt::Key_Meta || nextKey == Qt::Key_Alt ||
            nextKey == Qt::Key_Super_L || nextKey == Qt::Key_AltGr)
        return;

    nextKey |= translateModifiers(e->modifiers(), e->text());
    int k0 = m_keySequence[0];
    int k1 = m_keySequence[1];
    int k2 = m_keySequence[2];
    int k3 = m_keySequence[3];
    switch (m_num) {
        case 0: k0 = nextKey; k1 = 0; k2 = 0; k3 = 0; break;
        case 1: k1 = nextKey; k2 = 0; k3 = 0; break;
        case 2: k2 = nextKey; k3 = 0; break;
        case 3: k3 = nextKey; break;
        default: break;
    }
    ++m_num;
    if (m_num > 3)
        m_num = 0;
    m_keySequence = QKeySequence(k0, k1, k2, k3);
    m_lineEdit->setText(m_keySequence.toString(QKeySequence::NativeText));
    e->accept();
    emit keySequenceChanged(m_keySequence);
}

void Qtilities::CoreGui::QtKeySequenceEdit::setKeySequence(const QKeySequence &sequence)
{
    if (sequence == m_keySequence)
        return;
    m_num = 0;
    m_keySequence = sequence;
    m_lineEdit->setText(m_keySequence.toString(QKeySequence::NativeText));
}

QKeySequence Qtilities::CoreGui::QtKeySequenceEdit::keySequence() const
{
    return m_keySequence;
}

int Qtilities::CoreGui::QtKeySequenceEdit::translateModifiers(Qt::KeyboardModifiers state, const QString &text) const
{
    int result = 0;
    if ((state & Qt::ShiftModifier) && (text.size() == 0 || !text.at(0).isPrint() || text.at(0).isLetter() || text.at(0).isSpace()))
        result |= Qt::SHIFT;
    if (state & Qt::ControlModifier)
        result |= Qt::CTRL;
    if (state & Qt::MetaModifier)
        result |= Qt::META;
    if (state & Qt::AltModifier)
        result |= Qt::ALT;
    return result;
}

void Qtilities::CoreGui::QtKeySequenceEdit::focusInEvent(QFocusEvent *e)
{
    m_lineEdit->event(e);
    m_lineEdit->selectAll();
    QWidget::focusInEvent(e);
}

void Qtilities::CoreGui::QtKeySequenceEdit::focusOutEvent(QFocusEvent *e)
{
    m_num = 0;
    m_lineEdit->event(e);
    QWidget::focusOutEvent(e);
}

void Qtilities::CoreGui::QtKeySequenceEdit::keyPressEvent(QKeyEvent *e)
{
    handleKeyEvent(e);
    e->accept();
}

void Qtilities::CoreGui::QtKeySequenceEdit::keyReleaseEvent(QKeyEvent *e)
{
    m_lineEdit->event(e);
}

bool Qtilities::CoreGui::QtKeySequenceEdit::event(QEvent *e)
{
    if (e->type() == QEvent::Shortcut ||
            e->type() == QEvent::ShortcutOverride  ||
            e->type() == QEvent::KeyRelease) {
        e->accept();
        return true;
    }
    return QWidget::event(e);
}

