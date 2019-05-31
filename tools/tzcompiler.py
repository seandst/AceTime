#!/usr/bin/env python3
#
# Copyright 2018 Brian T. Park
#
# MIT License.
"""
Main driver for TZ Database compiler. The data processing pipeline looks like
this:
                      TZDB files
                          |
                          v
                      Extractor
                          |
                          v
                     Transformer
                    /     |    \
                   /      |     v
                  /       |     PythonGenerator
                 /        |            \
                /  InlineGenerator      v
               /          |            zone_infos.py
              /           |            zone_policies.py
             /           / \           zone_strings.py
            /           /   \
           /           v     \
          / BufSizeEstimator  |
         /     /              | \
        v     v               |  \
   ArduinoGenerator           |   v
         |                    |   Validator
         v                    |
 zone_infos.{h,cpp}           |
 zone_policies.{h,cpp}        |
 zone_strings.{h,cpp}         |
                              |
                              v
                         TestDataGenerator
                           /       \
                          v         v
          ArduinoValidation      PythonValidation
           Generator              Generator
               |                    |
               v                    v
      validation_data.{h,cpp}    validation_data.py
      validation_tests.cpp


ZoneSpecifier class is used by:
    * TestDataGenerator
    * Validator

ZoneSpecifier.main() uses:
    * zone_infos.py
    * zone_policies.py
files generated by PythonGenerator.

"""
import argparse
import logging
import sys

from extractor import Extractor
from transformer import Transformer
from argenerator import ArduinoGenerator
from pygenerator import PythonGenerator
from ingenerator import InlineGenerator
from bufestimator import BufSizeEstimator
from tdgenerator import TestDataGenerator
from arvalgenerator import ArduinoValidationGenerator
from pyvalgenerator import PythonValidationGenerator
from validator import Validator


def main():
    """Read the test data chunks from the STDIN and print them out. The ability
    to run this from the command line is intended mostly for testing purposes.

    Usage:
        tzcompiler.py [flags...]
    """
    # Configure command line flags.
    parser = argparse.ArgumentParser(description='Generate Zone Info.')

    # Extractor
    parser.add_argument(
        '--input_dir', help='Location of the input directory', required=True)

    # Transformer
    parser.add_argument(
        '--start_year',
        help='Start year of Zone Eras (default: 2000)',
        type=int,
        default=2000)
    parser.add_argument(
        '--until_year',
        help='Until year of Zone Eras (default: 2038)',
        type=int,
        default=2038)
    parser.add_argument(
        '--granularity',
        help='Retained time values (UNTIL, AT, SAVE, RULES) fields ' +
        'in seconds (default: 60 or 900)',
        type=int)
    parser.add_argument(
        '--strict',
        help='Remove zones and rules not aligned at granularity time boundary',
        action='store_true',
        default=False)

    # Flags for the TestDataGenerator. If not given (default 0), then
    # the validation_start_year will be set to start_year, and the
    # validation_until_year will be set to until_year.
    #
    # pytz cannot handle dates after the end of 32-bit Unix time_t type
    # (2038-01-19T03:14:07Z), see
    # https://answers.launchpad.net/pytz/+question/262216, so the
    # validation_until_year cannot be greater than 2038.
    parser.add_argument(
        '--validation_start_year',
        help='Start year of ZoneSpecifier validation (default: start_year)',
        type=int,
        default=0)
    parser.add_argument(
        '--validation_until_year',
        help='Until year of ZoneSpecifier validation (default: 2038)',
        type=int,
        default=0)

    # Data pipeline selectors
    parser.add_argument(
        '--zonedb', help='Generate ZoneDB files', action='store_true')
    parser.add_argument(
        '--unittest', help='Generate Unit Test files', action='store_true')
    parser.add_argument(
        '--validate',
        help='Validate both buffer size and test data',
        action='store_true')
    parser.add_argument(
        '--validate_buffer_size',
        help='Determine the maximum size of internal buffers ',
        action='store_true')
    parser.add_argument(
        '--validate_test_data',
        help='Validate the test data',
        action='store_true')

    # Target language selectors
    parser.add_argument(
        '--python', help='Generate Python files', action='store_true')
    parser.add_argument(
        '--arduino_basic',
        help='Generate Arduino files for BasicZoneSpecifier',
        action='store_true')
    parser.add_argument(
        '--arduino_extended',
        help='Generate Arduino files ExtendedZoneSpecifier',
        action='store_true')

    # C++ namespace names for arduino_basic or arduino_extended
    parser.add_argument(
        '--db_namespace',
        help='C++ namespace for the zonedb files (default: zonedb or zonedbx)')

    # Enable zone_strings.{h,cpp} if requested
    parser.add_argument(
        '--generate_zone_strings',
        help='Generate Arduino zone_strings.{h,cpp} files',
        action='store_true')

    # File generators
    parser.add_argument(
        '--tz_version', help='Version string of the TZ files', required=True)
    parser.add_argument(
        '--output_dir', help='Location of the output directory')

    # Validator
    parser.add_argument('--zone', help='Name of time zone to validate')
    parser.add_argument('--year', help='Year to validate', type=int)
    parser.add_argument(
        '--viewing_months',
        help='Number of months to use for calculations (13, 14, 36)',
        type=int,
        default=14)
    parser.add_argument(
        '--validate_dst_offset',
        help='Validate the DST offset as well as the total UTC offset',
        action="store_true")
    parser.add_argument(
        '--debug_validator',
        help='Enable debug output from Validator',
        action="store_true")
    parser.add_argument(
        '--debug_specifier',
        help='Enable debug output from ZoneSpecifier',
        action="store_true")
    parser.add_argument(
        '--in_place_transitions',
        help='Use in-place Transition array to determine Active Transitions',
        action="store_true")
    parser.add_argument(
        '--optimize_candidates',
        help='Optimize the candidate transitions',
        action='store_true')

    # Parse the command line arguments
    args = parser.parse_args()

    # Configure logging. This should normally be executed after the
    # parser.parse_args() because it allows us set the logging.level using a
    # flag.
    logging.basicConfig(level=logging.INFO)

    # Set the defaults for validation_start_year and validation_until_year
    # if they were not specified.
    validation_start_year = args.start_year if args.validation_start_year == 0 \
        else args.validation_start_year
    validation_until_year = args.until_year if args.validation_until_year == 0 \
        else args.validation_until_year

    # How the script was invoked
    invocation = ' '.join(sys.argv)

    # Select target language
    if args.arduino_basic:
        language = 'arduino_basic'
    elif args.arduino_extended:
        language = 'arduino_extended'
    elif args.python:
        language = 'python'
    else:
        raise Exception(
            'Must provide a language ' +
            '(--arduino_basic, --arduino_extended, --python)')

    # Determine zonedb namespace
    if args.db_namespace:
        db_namespace = args.db_namespace
    else:
        db_namespace = ''
        if args.arduino_basic: db_namespace = 'zonedb'
        if args.arduino_extended: db_namespace = 'zonedbx'

    # Define language dependent granularity if not overridden by flag
    if args.granularity:
        granularity = args.granularity
    else:
        if language in ['arduino_basic', 'arduino_extended']:
            granularity = 900
        else:
            granularity = 60
    logging.info('Using granularity: %d' % granularity)

    # Extract the TZ files
    logging.info('======== Extracting TZ Data files')
    extractor = Extractor(args.input_dir)
    extractor.parse()
    extractor.print_summary()

    # Transform the TZ zones and rules
    logging.info('======== Transforming Zones and Rules')
    logging.info('Extracting years [%d, %d)', args.start_year, args.until_year)
    # TODO: pass args.until_year to limit the size of the generated data
    # structures.
    transformer = Transformer(extractor.zones_map, extractor.rules_map,
        language, args.start_year, args.until_year, granularity, args.strict)
    transformer.transform()
    transformer.print_summary()

    # Generate internal versions of zone_infos and zone_policies
    # so that ZoneSpecifier can be created.
    logging.info('======== Generating inlined zone_infos and zone_policies')
    inline_generator = InlineGenerator(
        transformer.zones_map, transformer.rules_map)
    (zone_infos, zone_policies) = inline_generator.generate_maps()
    logging.info('zone_infos=%d; zone_policies=%d', len(zone_infos),
                 len(zone_policies))

    # Generate the buf_size estimates for each zone, between start_year and
    # until_year.
    logging.info('======== Estimating transition buffer sizes')
    logging.info('Checking years in [%d, %d)', args.start_year, args.until_year)
    estimator = BufSizeEstimator(zone_infos, zone_policies, args.start_year,
        args.until_year)
    (buf_sizes, max_size) = estimator.estimate()
    logging.info('Num zones=%d; Max buffer size=%d', len(buf_sizes), max_size)

    # Validate the zone_infos and zone_policies if requested
    validate_buffer_size = False
    validate_test_data = False
    if args.validate:
        validate_buffer_size = True
        validate_test_data = True
    if args.validate_buffer_size:
        validate_buffer_size = True
    if args.validate_test_data:
        validate_test_data = True

    if args.zonedb:
        # Create the Python or Arduino files if requested
        if not args.output_dir:
            logging.error('Must provide --output_dir to generate Python files')
            sys.exit(1)
        if language == 'python':
            logging.info('======== Creating Python zonedb files')
            generator = PythonGenerator(invocation, args.tz_version,
                                        Extractor.ZONE_FILES,
                                        transformer.zones_map,
                                        transformer.rules_map,
                                        transformer.all_removed_zones,
                                        transformer.all_removed_policies,
                                        transformer.all_notable_zones,
                                        transformer.all_notable_policies)
            generator.generate_files(args.output_dir)
        elif language == 'arduino_basic' or language == 'arduino_extended':
            extended = (language == 'arduino_extended')
            logging.info('======== Creating Arduino zonedb files')
            generator = ArduinoGenerator(
                invocation=invocation,
                tz_version=args.tz_version,
                tz_files=Extractor.ZONE_FILES,
                extended=extended,
                db_namespace=db_namespace,
                generate_zone_strings=args.generate_zone_strings,
                start_year=args.start_year,
                until_year=args.until_year,
                zones_map=transformer.zones_map,
                rules_map=transformer.rules_map,
                removed_zones=transformer.all_removed_zones,
                removed_policies=transformer.all_removed_policies,
                notable_zones=transformer.all_notable_zones,
                notable_policies=transformer.all_notable_policies,
                format_strings=transformer.format_strings,
                zone_strings=transformer.zone_strings,
                buf_sizes=buf_sizes)
            generator.generate_files(args.output_dir)
        else:
            raise Exception("Unrecognized language '%s'" % language)
    elif args.unittest:
        logging.info('======== Generating unit test files')

        # Generate test data for unit test.
        logging.info('Generating test data for years in [%d, %d)',
            validation_start_year, validation_until_year)
        data_generator = TestDataGenerator(zone_infos, zone_policies,
            granularity, validation_start_year, validation_until_year)
        (test_data, num_items) = data_generator.create_test_data()
        logging.info('Num zones=%d; Num test items=%d', len(test_data),
            num_items)

        # Generate validation data files
        logging.info('Generating test validation files')
        if language == 'arduino_basic' or language == 'arduino_extended':
            extended = (language == 'arduino_extended')
            arval_generator = ArduinoValidationGenerator(
                invocation, args.tz_version, test_data, num_items, extended)
            arval_generator.generate_files(args.output_dir)
        elif language == 'python':
            pyval_generator = PythonValidationGenerator(
                invocation, args.tz_version, test_data, num_items)
            pyval_generator.generate_files(args.output_dir)
        else:
            raise Exception("Unrecognized language '%s'" % language)
    elif validate_buffer_size or validate_test_data:
        validator = Validator(
            zone_infos=zone_infos,
            zone_policies=zone_policies,
            granularity=granularity,
            viewing_months=args.viewing_months,
            validate_dst_offset=args.validate_dst_offset,
            debug_validator=args.debug_validator,
            debug_specifier=args.debug_specifier,
            zone_name=args.zone,
            year=args.year,
            start_year=validation_start_year,
            until_year=validation_until_year,
            in_place_transitions=args.in_place_transitions,
            optimize_candidates=args.optimize_candidates)

        if validate_buffer_size:
            logging.info('======== Validating transition buffer sizes')
            validator.validate_buffer_size()

        if validate_test_data:
            logging.info('======== Validating test data')
            validator.validate_test_data()
    else:
        logging.error(
            'One of (--zonedb, --validate, --unittest) must be given')
        sys.exit(1)

    logging.info('======== Finished processing TZ Data files.')


if __name__ == '__main__':
    main()
