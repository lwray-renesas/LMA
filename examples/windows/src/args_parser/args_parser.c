#include "args_parser.h"
#include "string.h"

int Args_Parse(const int argc, const char *argv[], arg_data_t *p_arg_data)
{
  int arg_count = 0;

  /* Loop through all arguments*/
  for (int i = 0; i < argc; ++i)
  {
    /* Check it against every desired argument flag*/
    for (int arg = 0; arg < p_arg_data->args_count; ++arg)
    {
      /* If flag already associated with data, skip*/
      if (NULL == p_arg_data->p_args[arg].info)
      {
        /* If substring found at first index of argument input*/
        if (0 == memcmp(argv[i], p_arg_data->p_args[arg].flag, p_arg_data->p_args[arg].flag_length))
        {
          p_arg_data->p_args[arg].info = argv[i] + p_arg_data->p_args[arg].flag_length;
          ++arg_count;
        }
      }
    }
  }

  return arg_count;
}
