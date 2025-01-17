#ifndef _ARGS_PARSER_H_
#define _ARGS_PARSER_H_

/** @brief Struct defining the components of an argument */
typedef struct arg_t
{
  const char *const flag; /**< Pointer to string for the flag to look for. */
  const char *info; /**< Pointer to the contents of the argument if found (should be initialised to
                       NULL otherwise) */
  const size_t
      flag_length; /**< Length of the flag in bytes computed at compile time using sizeof */
} arg_t;

/** @brief Strcut defining the components of an argument set */
typedef struct arg_data_t
{
  arg_t *p_args;           /**< pointer to a set of arguments */
  const size_t args_count; /**< an argument count to show how many args are in the argument set */
} arg_data_t;

/** @brief Parses the passed arguments
 * @param[in] argc - number of argument passed.
 * @param[in] argv - array of strings (arguments).
 * @param[in] p_arg_data - pointer to argument information for the applicaiton.
 * @return number of arguments identified.
 */
int Args_Parse(const int argc, const char *argv[], arg_data_t *p_arg_data);

#endif /* _ARGS_PARSER_H_*/
