#include "FlattenedDirectoryViewModel.hpp"

namespace {
const std::vector<QString> tableHeaders = {QObject::tr("Name"), QObject::tr("Size"), QObject::tr("Date modified")};
}

int FlattenedDirectoryViewModel::rowCount(const QModelIndex&) const { return static_cast<int>(m_modelEntries.size()); }

QVariant FlattenedDirectoryViewModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (static_cast<unsigned>(section) >= tableHeaders.size())
        return {};

    switch (role) {
    case Qt::DisplayRole:
        return tableHeaders[section];
    default:
        break;
    }

    return {};
}

static std::optional<QString>
GetDisplayDataForColumn(const std::vector<FlattenedDirectoryViewModel::ModelEntry>& entries, int row, int column) {
    switch (column) {
    case 0:
        return entries[row].displayName;
    case 1:
        return entries[row].displaySize;
    case 2:
        return entries[row].displayDateModified;
    }
    return {};
}

QVariant FlattenedDirectoryViewModel::data(const QModelIndex& index, int role) const {
    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole:
        if (const auto& displayString = GetDisplayDataForColumn(m_modelEntries, index.row(), index.column())) {
            return *displayString;
        }
        return "";
    case Qt::UserRole:
        return m_modelEntries[index.row()].fullPath;
    case Qt::DecorationRole:
        if (index.column() == 0)
            return m_modelEntries[index.row()].icon;
    default:
        break;
    }

    return {};
}

void FlattenedDirectoryViewModel::ChangeDirectory(const QString&) {}

void FlattenedDirectoryViewModel::RefreshDirectory(const QString& dir) { ChangeDirectory(dir); }

std::optional<std::string> FlattenedDirectoryViewModel::ClaimError() { return {}; }
