#include "Query.h"

void QueryResult::do_or(const QueryResult &&other) {
    if (this->is_everything() || other.is_everything()) {
        has_everything = true;
        return;
    }

    std::vector<FileId> new_results;
    std::set_union(
            other.results.begin(), other.results.end(), results.begin(), results.end(),
            std::back_inserter(new_results));
    std::swap(new_results, results);
}

void QueryResult::do_and(const QueryResult &&other) {
    if (other.is_everything()) {
        return;
    }
    if (this->is_everything()) {
        *this = QueryResult(other);
        return;
    }

    auto new_end = std::set_intersection(
            other.results.begin(), other.results.end(), results.begin(), results.end(),
            results.begin());
    results.erase(new_end, results.end());
}

const std::vector<Query> &Query::as_queries() const {
    if (type != QueryType::AND && type != QueryType::OR) {
        throw std::runtime_error("This query doesn\'t contain subqueries.");
    }

    return queries;
}

Query::Query(const QueryType &type, const std::vector<Query> &queries)
    : type(type), queries(queries) {}

Query::Query(const QString &qstr) : type(QueryType::PRIMITIVE), value(qstr), queries() {}

bool Query::operator==(Query other) const {
    return type == other.type && value == other.value && queries == other.queries;
}

std::ostream &operator<<(std::ostream &os, const Query &query) {
    QueryType type = query.get_type();
    if (type == QueryType::AND || type == QueryType::OR) {
        if (type == QueryType::AND) {
            os << "AND(";
        } else if (type == QueryType::OR) {
            os << "OR(";
        }

        bool is_first = true;
        for (const auto &q : query.as_queries()) {
            if (!is_first) {
                os << ", ";
            }
            is_first = false;
            os << q;
        }
        os << ")";
    } else if (type == QueryType::PRIMITIVE) {
        os << "'" << query.as_string_repr() << "'";
    }
    return os;
}

const QueryType &Query::get_type() const { return type; }

const QString &Query::as_value() const {
    if (type != QueryType::PRIMITIVE) {
        throw std::runtime_error("This query doesn\'t have any value.");
    }

    return value;
}

std::string Query::as_string_repr() const {
    std::string out;

    for (const auto &token : as_value()) {
        if (token.type() == QTokenType::WILDCARD) {
            out += "\\x??";
        } else {
            out += token.val();
        }
    }

    return out;
}

Query q(const QString &qstr) { return Query(qstr); }

Query q_and(const std::vector<Query> &queries) { return Query(QueryType::AND, queries); }

Query q_or(const std::vector<Query> &queries) { return Query(QueryType::OR, queries); }
