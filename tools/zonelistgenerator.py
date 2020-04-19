# Copyright 2019 Brian T. Park
#
# MIT License

import logging
import os
from tzdb.tzdbgenerator import TzDb


class ZoneListGenerator:
    """Create a zones.txt file that contains the names of zones supported by
    zonedb and zonedbx. Will be used by external programs (e.g.
    TestDataGenerator.java or test_data_generator.cpp) to generate the
    validation_data.* files using the appropriate third party timezone library
    (e.g. java.time or Hinnant date library).
    """

    ZONES_FILE = """\
# This file was generated by the following script:
#
# $ {invocation}
#
# using the TZ Database files
#
#  {tz_files}
#
# from https://github.com/eggert/tz/releases/tag/{tz_version}
#
# DO NOT EDIT

# numZones: {numZones}
{zoneStrings}
"""

    def __init__(
        self,
        invocation: str,
        tzdb: TzDb,
    ):
        """
        Args:
            zones_map (dict): {full_name -> ZoneEra[]}
        """
        self.invocation = invocation
        self.tz_version = tzdb['tz_version']
        self.tz_files = tzdb['tz_files']
        self.scope = tzdb['scope']
        self.zones_map = tzdb['zones_map']

        self.file_name = 'zones.txt'

    def generate_files(self, output_dir: str) -> None:
        """Generate a text file that contains the list of zones.
        """
        self._write_file(output_dir, self.file_name, self._generate_zones())

    def _generate_zones(self) -> str:
        zone_strings = ""
        for name, eras in sorted(self.zones_map.items()):
            zone_strings += name + '\n'
        return self.ZONES_FILE.format(
            invocation=self.invocation,
            tz_version=self.tz_version,
            tz_files=', '.join(self.tz_files),
            scope=self.scope,
            zoneStrings=zone_strings,
            numZones=len(self.zones_map))

    def _write_file(self, output_dir: str, filename: str, content: str) -> None:
        full_filename = os.path.join(output_dir, filename)
        with open(full_filename, 'w', encoding='utf-8') as output_file:
            print(content, end='', file=output_file)
        logging.info("Created %s", full_filename)
