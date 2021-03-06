
#ifndef LISP_TOKENIZER_HPP
#define LISP_TOKENIZER_HPP

#include <stdexcept>

#include <boost/regex.hpp>


namespace lisp {
    /**
       @brief Is thrown after an error occured during parsing.
       (e.g a syntactical error done by the user.)
    */
    class parse_error : public std::runtime_error
    {
    public:
        parse_error(const std::string& what, int line)
            : std::runtime_error(what),
              m_line(line)
            {
            }

        int line() const
            {
                return m_line;
            }

    private:
        int m_line;
    };

    enum token {
        END,
        LEFT_PARENTHESIS,
        RIGHT_PARENTHESIS,
        STRING,
        SYMBOL,
        NUMBER,
        DOT,
        QUOTE
    };

    /**
       Parses an input stream using an iterator and cuts it into
       tokens which can be consumed and processed by the interpreter
       or compiler.
    */
    template <class T>
    class tokenizer
    {
    public:
        tokenizer(T& iterator, const T& end = T())
            : m_iterator(iterator),
              m_end(end),
              m_line(1),
              m_current_token(END)
            {
            }

        token next_token()
            {
                m_cache.clear();

                if(m_iterator == m_end)
                    return END;

                char last = '\0';

                for(; m_iterator != m_end; ++m_iterator) {
                    switch(*m_iterator) {
                    case '(':
                        m_cache += *m_iterator;
                        ++m_iterator;
                        return (m_current_token = LEFT_PARENTHESIS);
                    case ')':
                        m_cache += *m_iterator;
                        ++m_iterator;
                        return (m_current_token = RIGHT_PARENTHESIS);
                    case '"':
                        parse_string();
                        return (m_current_token = STRING);
                    case '.':
                        m_cache += *m_iterator;
                        ++m_iterator;
                        return (m_current_token = DOT);
                    case '\'':
                        m_cache += *m_iterator;
                        ++m_iterator;
                        return (m_current_token = QUOTE);
                    case '\n':
                        ++m_line;
                    case ' ':
                    case '\t':
                        last = *m_iterator;
                        break;
                    default:
                        return parse_symbol_or_number();
                    }
                }

                if(m_iterator == m_end && (last == ' ' ||
                                           last == '\t' ||
                                           last == '\n'))
                    return END;

                error("unexpected end of file");

                assert(false);
            }

        token current_token() const
            {
                return m_current_token;
            }

        const std::string& value()
            {
                return m_cache;
            }

        void parse_string()
            {
                bool escaped;

                assert(*m_iterator == '"');
                ++m_iterator;

                for(; m_iterator != m_end; ++m_iterator) {
                    switch(*m_iterator) {
                    case '\\': {
                        if(escaped) {
                            m_cache += *m_iterator;
                            escaped = false;
                        }
                        else
                            escaped = true;

                        break;
                    }
                    case '"': {
                        if(escaped) {
                            m_cache += *m_iterator;
                            escaped = false;
                        }
                        else {
                            ++m_iterator;
                            return;
                        }

                        break;
                    }
                    default: {
                        if(escaped) {
                            escaped = false;

                            switch(*m_iterator) {
                            case 'n':
                                m_cache += '\n';
                                break;
                            case 't':
                                m_cache += '\t';
                                break;
                            default:
                                warn("unknown escape sequence: \\" + *m_iterator);
                            }
                        }
                        else
                            m_cache += *m_iterator;

                        break;
                    }
                    }
                }

                error("unexpected end of file in string");
            }

        token parse_symbol_or_number()
            {
                for(; m_iterator != m_end && *m_iterator != ' ' &&
                        *m_iterator != '\t' && *m_iterator != '\n' &&
                        *m_iterator != '"' && *m_iterator != '\'' &&
                        *m_iterator != '(' && *m_iterator != ')'; ++m_iterator)

                    m_cache += *m_iterator;


		boost::regex number_regex("^\\-?[0-9]+([\\./][0-9]+)?$");

		if(boost::regex_match(m_cache, number_regex))
		    return (m_current_token = NUMBER);

		return (m_current_token = SYMBOL);
            }

        int line() const
            {
                return m_line;
            }

    protected:
        void warn(const std::string& msg) 
            {
                std::cout << "Warning:" << m_line << ": " << msg << std::endl;
            }

        void error(const std::string& msg)
            {
                throw parse_error(msg, m_line);
            }

    private:
        T& m_iterator;
        T m_end;
        int m_line;
        std::string m_cache;
        token m_current_token;
    };
}

#endif  // LISP_TOKENIZER_HPP
