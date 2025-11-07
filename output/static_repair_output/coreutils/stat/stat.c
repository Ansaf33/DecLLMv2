// Function: human_fstype
char * human_fstype(long param_1) {
  uint uVar1;
  
  uVar1 = *(uint *)(param_1 + 0x58);
  if (uVar1 == 0xff534d42) {
    return "cifs";
  }
  if (uVar1 < 0xff534d43) {
    if (uVar1 == 0xfe534d42) {
      return "smb2";
    }
    if (uVar1 < 0xfe534d43) {
      if (uVar1 == 0xf995e849) {
        return "hpfs";
      }
      if (uVar1 < 0xf995e84a) {
        if (uVar1 == 0xf97cff8c) {
          return "selinux";
        }
        if (uVar1 < 0xf97cff8d) {
          if (uVar1 == 0xf2f52010) {
            return "f2fs";
          }
          if (uVar1 < 0xf2f52011) {
            if (uVar1 == 0xe0f5e1e2) {
              return "erofs";
            }
            if (uVar1 < 0xe0f5e1e3) {
              if (uVar1 == 0xde5e81e4) {
                return "efivarfs";
              }
              if (uVar1 < 0xde5e81e5) {
                if (uVar1 == 0xcafe4a11) {
                  return "bpf_fs";
                }
                if (uVar1 < 0xcafe4a12) {
                  if (uVar1 == 0xca451a4e) {
                    return "bcachefs";
                  }
                  if (uVar1 < 0xca451a4f) {
                    if (uVar1 == 0xc97e8168) {
                      return "logfs";
                    }
                    if (uVar1 < 0xc97e8169) {
                      if (uVar1 == 0xc7571590) {
                        return "ppc-cmm-fs";
                      }
                      if (uVar1 < 0xc7571591) {
                        if (uVar1 == 0xbeefdead) {
                          return "snfs";
                        }
                        if (uVar1 < 0xbeefdeae) {
                          if (uVar1 == 0xbacbacbc) {
                            return "vmhgfs";
                          }
                          if (uVar1 < 0xbacbacbd) {
                            if (uVar1 == 0xabba1974) {
                              return "xenfs";
                            }
                            if (uVar1 < 0xabba1975) {
                              if (uVar1 == 0xaad7aaea) {
                                return "panfs";
                              }
                              if (uVar1 < 0xaad7aaeb) {
                                if (uVar1 == 0xa501fcf5) {
                                  return "vxfs";
                                }
                                if (uVar1 < 0xa501fcf6) {
                                  if (uVar1 == 0x958458f6) {
                                    return "hugetlbfs";
                                  }
                                  if (uVar1 < 0x958458f7) {
                                    if (uVar1 == 0x9123683e) {
                                      return "btrfs";
                                    }
                                    if (uVar1 < 0x9123683f) {
                                      if (uVar1 == 0x858458f6) {
                                        return "ramfs";
                                      }
                                      if (uVar1 < 0x858458f7) {
                                        if (uVar1 == 0x7c7c6673) {
                                          return "prl_fs";
                                        }
                                        if (uVar1 < 0x7c7c6674) {
                                          if (uVar1 == 0x794c7630) {
                                            return "overlayfs";
                                          }
                                          if (uVar1 < 0x794c7631) {
                                            if (uVar1 == 0x786f4256) {
                                              return "vboxsf";
                                            }
                                            if (uVar1 < 0x786f4257) {
                                              if (uVar1 == 0x74726163) {
                                                return "tracefs";
                                              }
                                              if (uVar1 < 0x74726164) {
                                                if (uVar1 == 0x7461636f) {
                                                  return "ocfs2";
                                                }
                                                if (uVar1 < 0x74616370) {
                                                  if (uVar1 == 0x73757245) {
                                                    return "coda";
                                                  }
                                                  if (uVar1 < 0x73757246) {
                                                    if (uVar1 == 0x73727279) {
                                                      return "btrfs_test";
                                                    }
                                                    if (uVar1 < 0x7372727a) {
                                                      if (uVar1 == 0x73717368) {
                                                        return "squashfs";
                                                      }
                                                      if (uVar1 < 0x73717369) {
                                                        if (uVar1 == 0x73636673) {
                                                          return "securityfs";
                                                        }
                                                        if (uVar1 < 0x73636674) {
                                                          if (uVar1 == 0x6e736673) {
                                                            return "nsfs";
                                                          }
                                                          if (uVar1 < 0x6e736674) {
                                                            if (uVar1 == 0x6e667364) {
                                                              return "nfsd";
                                                            }
                                                            if (uVar1 < 0x6e667365) {
                                                              if (uVar1 == 0x6c6f6f70) {
                                                                return "binderfs";
                                                              }
                                                              if (uVar1 < 0x6c6f6f71) {
                                                                if (uVar1 == 0x6b414653) {
                                                                  return "k-afs";
                                                                }
                                                                if (uVar1 < 0x6b414654) {
                                                                  if (uVar1 == 0x68191122) {
                                                                    return "qnx6";
                                                                  }
                                                                  if (uVar1 < 0x68191123) {
                                                                    if (uVar1 == 0x67596969) {
                                                                      return "rpc_pipefs";
                                                                    }
                                                                    if (uVar1 < 0x6759696a) {
                                                                      if (uVar1 == 0x65735546) {
                                                                        return "fuse";
                                                                      }
                                                                      if (uVar1 < 0x65735547) {
                                                                        if (uVar1 == 0x65735543) {
                                                                          return "fusectl";
                                                                        }
                                                                        if (uVar1 < 0x65735544) {
                                                                          if (uVar1 == 0x64646178) {
                                                                            return "daxfs";
                                                                          }
                                                                          if (uVar1 < 0x64646179) {
                                                                            if (uVar1 == 0x64626720)
                                                                            {
                                                                              return "debugfs";
                                                                            }
                                                                            if (uVar1 < 0x64626721)
                                                                            {
                                                                              if (uVar1 == 
                                                  0x63677270) {
                                                    return "cgroup2fs";
                                                  }
                                                  if (uVar1 < 0x63677271) {
                                                    if (uVar1 == 0x62656572) {
                                                      return "sysfs";
                                                    }
                                                    if (uVar1 < 0x62656573) {
                                                      if (uVar1 == 0x62656570) {
                                                        return "configfs";
                                                      }
                                                      if (uVar1 < 0x62656571) {
                                                        if (uVar1 == 0x62646576) {
                                                          return "bdevfs";
                                                        }
                                                        if (uVar1 < 0x62646577) {
                                                          if (uVar1 == 0x61756673) {
                                                            return "aufs";
                                                          }
                                                          if (uVar1 < 0x61756674) {
                                                            if (uVar1 == 0x6165676c) {
                                                              return "pstorefs";
                                                            }
                                                            if (uVar1 < 0x6165676d) {
                                                              if (uVar1 == 0x61636673) {
                                                                return "acfs";
                                                              }
                                                              if (uVar1 < 0x61636674) {
                                                                if (uVar1 == 0x5dca2df5) {
                                                                  return "sdcardfs";
                                                                }
                                                                if (uVar1 < 0x5dca2df6) {
                                                                  if (uVar1 == 0x5a4f4653) {
                                                                    return "zonefs";
                                                                  }
                                                                  if (uVar1 < 0x5a4f4654) {
                                                                    if (uVar1 == 0x5a3c69f0) {
                                                                      return "aafs";
                                                                    }
                                                                    if (uVar1 < 0x5a3c69f1) {
                                                                      if (uVar1 == 0x58465342) {
                                                                        return "xfs";
                                                                      }
                                                                      if (uVar1 < 0x58465343) {
                                                                        if (uVar1 == 0x58295829) {
                                                                          return "zsmallocfs";
                                                                        }
                                                                        if (uVar1 < 0x5829582a) {
                                                                          if (uVar1 == 0x565a4653) {
                                                                            return "vzfs";
                                                                          }
                                                                          if (uVar1 < 0x565a4654) {
                                                                            if (uVar1 == 0x54190100)
                                                                            {
                                                                              return "ufs";
                                                                            }
                                                                            if (uVar1 < 0x54190101)
                                                                            {
                                                                              if (uVar1 == 
                                                  0x534f434b) {
                                                    return "sockfs";
                                                  }
                                                  if (uVar1 < 0x534f434c) {
                                                    if (uVar1 == 0x5346544e) {
                                                      return "ntfs";
                                                    }
                                                    if (uVar1 < 0x5346544f) {
                                                      if (uVar1 == 0x53464846) {
                                                        return "wslfs";
                                                      }
                                                      if (uVar1 < 0x53464847) {
                                                        if (uVar1 == 0x5346414f) {
                                                          return "afs";
                                                        }
                                                        if (uVar1 < 0x53464150) {
                                                          if (uVar1 == 0x5346314d) {
                                                            return "m1fs";
                                                          }
                                                          if (uVar1 < 0x5346314e) {
                                                            if (uVar1 == 0x5345434d) {
                                                              return "secretmem";
                                                            }
                                                            if (uVar1 < 0x5345434e) {
                                                              if (uVar1 == 0x52654973) {
                                                                return "reiserfs";
                                                              }
                                                              if (uVar1 < 0x52654974) {
                                                                if (uVar1 == 0x50495045) {
                                                                  return "pipefs";
                                                                }
                                                                if (uVar1 < 0x50495046) {
                                                                  if (uVar1 == 0x50494446) {
                                                                    return "pidfs";
                                                                  }
                                                                  if (uVar1 < 0x50494447) {
                                                                    if (uVar1 == 0x47504653) {
                                                                      return "gpfs";
                                                                    }
                                                                    if (uVar1 < 0x47504654) {
                                                                      if (uVar1 == 0x45584653) {
                                                                        return "exfs";
                                                                      }
                                                                      if (uVar1 < 0x45584654) {
                                                                        if (uVar1 == 0x454d444d) {
                                                                          return "devmem";
                                                                        }
                                                                        if (uVar1 < 0x454d444e) {
                                                                          if (uVar1 == 0x453dcd28) {
                                                                            return "cramfs-wend";
                                                                          }
                                                                          if (uVar1 < 0x453dcd29) {
                                                                            if (uVar1 == 0x444d4142)
                                                                            {
                                                                              return "dma-buf-fs";
                                                                            }
                                                                            if (uVar1 < 0x444d4143)
                                                                            {
                                                                              if (uVar1 == 
                                                  0x43415d53) {
                                                    return "smackfs";
                                                  }
                                                  if (uVar1 < 0x43415d54) {
                                                    if (uVar1 == 0x42494e4d) {
                                                      return "binfmt_misc";
                                                    }
                                                    if (uVar1 < 0x42494e4e) {
                                                      if (uVar1 == 0x42465331) {
                                                        return "befs";
                                                      }
                                                      if (uVar1 < 0x42465332) {
                                                        if (uVar1 == 0x3153464a) {
                                                          return "jfs";
                                                        }
                                                        if (uVar1 < 0x3153464b) {
                                                          if (uVar1 == 0x2fc12fc1) {
                                                            return "zfs";
                                                          }
                                                          if (uVar1 < 0x2fc12fc2) {
                                                            if (uVar1 == 0x2bad1dea) {
                                                              return "inotifyfs";
                                                            }
                                                            if (uVar1 < 0x2bad1deb) {
                                                              if (uVar1 == 0x28cd3d45) {
                                                                return "cramfs";
                                                              }
                                                              if (uVar1 < 0x28cd3d46) {
                                                                if (uVar1 == 0x24051905) {
                                                                  return "ubifs";
                                                                }
                                                                if (uVar1 < 0x24051906) {
                                                                  if (uVar1 == 0x2011bab0) {
                                                                    return "exfat";
                                                                  }
                                                                  if (uVar1 < 0x2011bab1) {
                                                                    if (uVar1 == 0x1badface) {
                                                                      return "bfs";
                                                                    }
                                                                    if (uVar1 < 0x1badfacf) {
                                                                      if (uVar1 == 0x19830326) {
                                                                        return "fhgfs";
                                                                      }
                                                                      if (uVar1 < 0x19830327) {
                                                                        if (uVar1 == 0x19800202) {
                                                                          return "mqueue";
                                                                        }
                                                                        if (uVar1 < 0x19800203) {
                                                                          if (uVar1 == 0x15013346) {
                                                                            return "udf";
                                                                          }
                                                                          if (uVar1 < 0x15013347) {
                                                                            if (uVar1 == 0x13661366)
                                                                            {
                                                                              return 
                                                  "balloon-kvm-fs";
                                                  }
                                                  if (uVar1 < 0x13661367) {
                                                    if (uVar1 == 0x11307854) {
                                                      return "inodefs";
                                                    }
                                                    if (uVar1 < 0x11307855) {
                                                      if (uVar1 == 0xbd00bd0) {
                                                        return "lustre";
                                                      }
                                                      if (uVar1 < 0xbd00bd1) {
                                                        if (uVar1 == 0xbad1dea) {
                                                          return "futexfs";
                                                        }
                                                        if (uVar1 < 0xbad1deb) {
                                                          if (uVar1 == 0x9041934) {
                                                            return "anon-inode FS";
                                                          }
                                                          if (uVar1 < 0x9041935) {
                                                            if (uVar1 == 0x7655821) {
                                                              return "rdt";
                                                            }
                                                            if (uVar1 < 0x7655822) {
                                                              if (uVar1 == 0x13111a8) {
                                                                return "ibrix";
                                                              }
                                                              if (uVar1 < 0x13111a9) {
                                                                if (uVar1 == 0x12ff7b7) {
                                                                  return "coh";
                                                                }
                                                                if (uVar1 < 0x12ff7b8) {
                                                                  if (uVar1 == 0x12ff7b6) {
                                                                    return "sysv2";
                                                                  }
                                                                  if (uVar1 < 0x12ff7b7) {
                                                                    if (uVar1 == 0x12ff7b5) {
                                                                      return "sysv4";
                                                                    }
                                                                    if (uVar1 < 0x12ff7b6) {
                                                                      if (uVar1 == 0x12ff7b4) {
                                                                        return "xenix";
                                                                      }
                                                                      if (uVar1 < 0x12ff7b5) {
                                                                        if (uVar1 == 0x12fd16d) {
                                                                          return "xia";
                                                                        }
                                                                        if (uVar1 < 0x12fd16e) {
                                                                          if (uVar1 == 0x1161970) {
                                                                            return "gfs/gfs2";
                                                                          }
                                                                          if (uVar1 < 0x1161971) {
                                                                            if (uVar1 == 0x1021997)
                                                                            {
                                                                              return "v9fs";
                                                                            }
                                                                            if (uVar1 < 0x1021998) {
                                                                              if (uVar1 == 0x1021994
                                                                                 ) {
                                                                                return "tmpfs";
                                                                              }
                                                                              if (uVar1 < 0x1021995)
                                                                              {
                                                                                if (uVar1 == 
                                                  0xc36400) {
                                                    return "ceph";
                                                  }
                                                  if (uVar1 < 0xc36401) {
                                                    if (uVar1 == 0xc0ffee) {
                                                      return "hostfs";
                                                    }
                                                    if (uVar1 < 0xc0ffef) {
                                                      if (uVar1 == 0x414a53) {
                                                        return "efs";
                                                      }
                                                      if (uVar1 < 0x414a54) {
                                                        if (uVar1 == 0x27e0eb) {
                                                          return "cgroupfs";
                                                        }
                                                        if (uVar1 < 0x27e0ec) {
                                                          if (uVar1 == 0x11954) {
                                                            return "ufs";
                                                          }
                                                          if (uVar1 < 0x11955) {
                                                            if (uVar1 == 0xf15f) {
                                                              return "ecryptfs";
                                                            }
                                                            if (uVar1 < 0xf160) {
                                                              if (uVar1 == 0xef53) {
                                                                return "ext2/ext3";
                                                              }
                                                              if (uVar1 < 0xef54) {
                                                                if (uVar1 == 0xef51) {
                                                                  return "ext2";
                                                                }
                                                                if (uVar1 < 0xef52) {
                                                                  if (uVar1 == 0xadff) {
                                                                    return "affs";
                                                                  }
                                                                  if (uVar1 < 0xae00) {
                                                                    if (uVar1 == 0xadf5) {
                                                                      return "adfs";
                                                                    }
                                                                    if (uVar1 < 0xadf6) {
                                                                      if (uVar1 == 0x9fa2) {
                                                                        return "usbdevfs";
                                                                      }
                                                                      if (uVar1 < 0x9fa3) {
                                                                        if (uVar1 == 0x9fa1) {
                                                                          return "openprom";
                                                                        }
                                                                        if (uVar1 < 0x9fa2) {
                                                                          if (uVar1 == 0x9fa0) {
                                                                            return "proc";
                                                                          }
                                                                          if (uVar1 < 0x9fa1) {
                                                                            if (uVar1 == 0x9660) {
                                                                              return "isofs";
                                                                            }
                                                                            if (uVar1 < 0x9661) {
                                                                              if (uVar1 == 0x72b6) {
                                                                                return "jffs2";
                                                                              }
                                                                              if (uVar1 < 0x72b7) {
                                                                                if (uVar1 == 0x7275)
                                                                                {
                                                                                  return "romfs";
                                                                                }
                                                                                if (uVar1 < 0x7276)
                                                                                {
                                                                                  if (uVar1 == 
                                                  0x6969) {
                                                    return "nfs";
                                                  }
                                                  if (uVar1 < 0x696a) {
                                                    if (uVar1 == 0x5df5) {
                                                      return "exofs";
                                                    }
                                                    if (uVar1 < 0x5df6) {
                                                      if (uVar1 == 0x564c) {
                                                        return "novell";
                                                      }
                                                      if (uVar1 < 0x564d) {
                                                        if (uVar1 == 0x517b) {
                                                          return "smb";
                                                        }
                                                        if (uVar1 < 0x517c) {
                                                          if (uVar1 == 0x4d5a) {
                                                            return "minix3";
                                                          }
                                                          if (uVar1 < 0x4d5b) {
                                                            if (uVar1 == 0x4d44) {
                                                              return "msdos";
                                                            }
                                                            if (uVar1 < 0x4d45) {
                                                              if (uVar1 == 0x4858) {
                                                                return "hfsx";
                                                              }
                                                              if (uVar1 < 0x4859) {
                                                                if (uVar1 == 0x482b) {
                                                                  return "hfs+";
                                                                }
                                                                if (uVar1 < 0x482c) {
                                                                  if (uVar1 == 0x4244) {
                                                                    return "hfs";
                                                                  }
                                                                  if (uVar1 < 0x4245) {
                                                                    if (uVar1 == 0x4006) {
                                                                      return "fat";
                                                                    }
                                                                    if (uVar1 < 0x4007) {
                                                                      if (uVar1 == 0x4004) {
                                                                        return "isofs";
                                                                      }
                                                                      if (uVar1 < 0x4005) {
                                                                        if (uVar1 == 0x4000) {
                                                                          return "isofs";
                                                                        }
                                                                        if (uVar1 < 0x4001) {
                                                                          if (uVar1 == 0x3434) {
                                                                            return "nilfs";
                                                                          }
                                                                          if (uVar1 < 0x3435) {
                                                                            if (uVar1 == 0x2478) {
                                                                              return 
                                                  "minix v2 (30 char.)";
                                                  }
                                                  if (uVar1 < 0x2479) {
                                                    if (uVar1 == 0x2468) {
                                                      return "minix v2";
                                                    }
                                                    if (uVar1 < 0x2469) {
                                                      if (uVar1 == 0x1cd1) {
                                                        return "devpts";
                                                      }
                                                      if (uVar1 < 0x1cd2) {
                                                        if (uVar1 == 0x138f) {
                                                          return "minix (30 char.)";
                                                        }
                                                        if (uVar1 < 0x1390) {
                                                          if (uVar1 == 0x137f) {
                                                            return "minix";
                                                          }
                                                          if (uVar1 < 0x1380) {
                                                            if (uVar1 == 0x137d) {
                                                              return "ext";
                                                            }
                                                            if (uVar1 < 0x137e) {
                                                              if (uVar1 == 0x1373) {
                                                                return "devfs";
                                                              }
                                                              if (uVar1 < 0x1374) {
                                                                if (uVar1 == 0x7c0) {
                                                                  return "jffs";
                                                                }
                                                                if (uVar1 < 0x7c1) {
                                                                  if (uVar1 == 0x187) {
                                                                    return "autofs";
                                                                  }
                                                                  if (uVar1 < 0x188) {
                                                                    if (uVar1 == 0x2f) {
                                                                      return "qnx4";
                                                                    }
                                                                    if (uVar1 == 0x33) {
                                                                      return "z3fold";
                                                                    }
                                                                  }
                                                                }
                                                              }
                                                            }
                                                          }
                                                        }
                                                      }
                                                    }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                  }
                                                }
                                              }
                                            }
                                          }
                                        }
                                      }
                                    }
                                  }
                                }
                              }
                            }
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  sprintf(buf_5,"UNKNOWN (0x%lx)",(ulong)*(uint *)(param_1 + 0x58));
  return buf_5;
}

// Function: human_access
undefined1 * human_access(undefined8 param_1) {
  filemodestring(param_1,modebuf_4);
  modebuf_4[10] = 0;
  return modebuf_4;
}

// Function: human_time
undefined1 * human_time(undefined8 param_1,undefined8 param_2) {
  char *pcVar1;
  long lVar2;
  undefined8 uVar3;
  long in_FS_OFFSET;
  undefined8 local_88;
  undefined8 local_80;
  uint local_6c;
  undefined local_68 [64];
  undefined local_28 [24];
  long local_10;
  
  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  local_88 = param_1;
  local_80 = param_2;
  if (tz_3 == 0) {
    pcVar1 = getenv("TZ");
    tz_3 = tzalloc(pcVar1);
  }
  local_6c = (uint)local_80;
  lVar2 = localtime_rz(tz_3,&local_88,local_68);
  if (lVar2 == 0) {
    uVar3 = timetostr(local_88,local_28);
    sprintf(str_2,"%s.%09d",uVar3,(ulong)local_6c);
  }
  else {
    nstrftime(str_2,0x3d,"%Y-%m-%d %H:%M:%S.%N %z",local_68,tz_3,local_6c);
  }
  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return str_2;
}

// Function: make_format
void make_format(long param_1,long param_2,char *param_3,char *param_4) {
  char *pcVar1;
  char *local_20;
  char *local_18;
  
  local_20 = (char *)(param_1 + 1);
  local_18 = local_20;
  while ((local_18 < (char *)(param_2 + param_1) &&
         (pcVar1 = strchr("\'-+ #0I",(int)*local_18), pcVar1 != (char *)0x0))) {
    pcVar1 = strchr(param_3,(int)*local_18);
    if (pcVar1 != (char *)0x0) {
      *local_20 = *local_18;
      local_20 = local_20 + 1;
    }
    local_18 = local_18 + 1;
  }
  while (local_18 < (char *)(param_2 + param_1)) {
    *local_20 = *local_18;
    local_20 = local_20 + 1;
    local_18 = local_18 + 1;
  }
  strcpy(local_20,param_4);
  return;
}

// Function: out_string
void out_string(char *param_1,undefined8 param_2,undefined8 param_3) {
  make_format(param_1,param_2,&DAT_00104afb,&DAT_00104af9);
  printf(param_1,param_3);
  return;
}

// Function: out_int
void out_int(char *param_1,undefined8 param_2,undefined8 param_3) {
  make_format(param_1,param_2,"\'-+ 0",&DAT_00104afd);
  printf(param_1,param_3);
  return;
}

// Function: out_uint
void out_uint(char *param_1,undefined8 param_2,undefined8 param_3) {
  make_format(param_1,param_2,&DAT_00104b09,&DAT_00104b06);
  printf(param_1,param_3);
  return;
}

// Function: out_uint_o
void out_uint_o(char *param_1,undefined8 param_2,undefined8 param_3) {
  make_format(param_1,param_2,&DAT_00104b10,&DAT_00104b0d);
  printf(param_1,param_3);
  return;
}

// Function: out_uint_x
void out_uint_x(char *param_1,undefined8 param_2,undefined8 param_3) {
  make_format(param_1,param_2,&DAT_00104b10,&DAT_00104b14);
  printf(param_1,param_3);
  return;
}

// Function: out_minus_zero
void out_minus_zero(char *param_1,undefined8 param_2) {
  make_format(param_1,param_2,"\'-+ 0",&DAT_00104b17);
  printf(param_1,DAT_00106040);
  return;
}

// Function: out_epoch_sec
void out_epoch_sec(char *param_1,char *param_2,long param_3,long param_4) {
  char *pcVar1;
  char cVar2;
  int iVar3;
  uint uVar4;
  long lVar5;
  uint uVar6;
  bool bVar7;
  long local_a8;
  int local_84;
  uint local_80;
  int local_7c;
  uint local_78;
  uint local_74;
  char *local_50;
  char *local_48;
  char *local_40;
  char *local_38;
  
  local_48 = (char *)memchr(param_1,0x2e,(size_t)param_2);
  local_84 = 0;
  local_80 = 0;
  bVar7 = false;
  local_50 = param_2;
  if (local_48 != (char *)0x0) {
    local_50 = local_48 + -(long)param_1;
    param_1[(long)param_2] = '\0';
    cVar2 = c_isdigit((int)local_48[1]);
    if (cVar2 == '\0') {
      local_80 = 9;
    }
    else {
      lVar5 = __isoc23_strtol(local_48 + 1,0,10);
      if (0x7fffffff < lVar5) {
        lVar5 = 0x7fffffff;
      }
      local_80 = (uint)lVar5;
    }
    if ((local_80 != 0) && (cVar2 = c_isdigit((int)local_48[-1]), cVar2 != '\0')) {
      *local_48 = '\0';
      do {
        pcVar1 = local_48 + -1;
        cVar2 = c_isdigit((int)local_48[-2]);
        local_48 = pcVar1;
      } while (cVar2 != '\0');
      lVar5 = __isoc23_strtol(pcVar1,0,10);
      if (0x7fffffff < lVar5) {
        lVar5 = 0x7fffffff;
      }
      local_84 = (int)lVar5;
      if (1 < local_84) {
        cVar2 = *pcVar1;
        local_50 = pcVar1 + (cVar2 == '0') + -(long)param_1;
        if (decimal_point_len < (ulong)(long)local_84) {
          iVar3 = local_84 - (int)decimal_point_len;
        }
        else {
          iVar3 = 0;
        }
        if ((1 < iVar3) && (local_40 = param_1, local_38 = param_1, 1 < (int)(iVar3 - local_80))) {
          for (; local_38 < pcVar1 + (cVar2 == '0'); local_38 = local_38 + 1) {
            if (*local_38 == '-') {
              bVar7 = true;
            }
            else {
              *local_40 = *local_38;
              local_40 = local_40 + 1;
            }
          }
          if (bVar7) {
            lVar5 = 0;
          }
          else {
            iVar3 = sprintf(local_40,"%d",(ulong)(iVar3 - local_80));
            lVar5 = (long)iVar3;
          }
          local_50 = local_40 + (lVar5 - (long)param_1);
        }
      }
    }
  }
  local_7c = 1;
  for (local_78 = local_80; (int)local_78 < 9; local_78 = local_78 + 1) {
    local_7c = local_7c * 10;
  }
  local_74 = (uint)(param_4 / (long)local_7c);
  bVar7 = false;
  local_a8 = param_3;
  if ((param_3 < 0) && (param_4 != 0)) {
    local_74 = ((int)(1000000000 / (long)local_7c) - local_74) -
               (uint)(param_4 % (long)local_7c != 0);
    local_a8 = (ulong)(local_74 != 0) + param_3;
    bVar7 = local_a8 == 0;
  }
  if (bVar7) {
    iVar3 = out_minus_zero(param_1,local_50);
  }
  else {
    iVar3 = out_int(param_1,local_50,local_a8);
  }
  if (local_80 != 0) {
    uVar4 = local_80;
    if (9 < (int)local_80) {
      uVar4 = 9;
    }
    if (iVar3 < 0) {
      iVar3 = 0;
    }
    if ((iVar3 < local_84) && (decimal_point_len < (ulong)(long)(local_84 - iVar3))) {
      uVar6 = ((local_84 - iVar3) - (int)decimal_point_len) - uVar4;
    }
    else {
      uVar6 = 0;
    }
    printf("%s%.*d%-*.*d",decimal_point,(ulong)uVar4,(ulong)local_74,(ulong)uVar6,
           (ulong)(local_80 - uVar4),0);
  }
  return;
}

// Function: out_file_context
bool out_file_context(char *param_1,long param_2,undefined8 param_3) {
  char cVar1;
  char extraout_var;
  char extraout_var_00;
  undefined8 uVar2;
  undefined8 uVar3;
  int *piVar4;
  undefined *puVar5;
  long in_FS_OFFSET;
  undefined *local_30;
  undefined8 local_28;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  if (follow_links == '\0') {
    lgetfilecon(param_3,&local_30);
    cVar1 = extraout_var_00;
  }
  else {
    getfilecon(param_3,&local_30);
    cVar1 = extraout_var;
  }
  if (cVar1 < '\0') {
    uVar2 = quotearg_style(4,param_3);
    uVar3 = gettext("failed to get security context of %s");
    piVar4 = __errno_location();
    local_28 = uVar3;
    error(0,*piVar4,uVar3,uVar2);
    local_30 = (undefined *)0x0;
  }
  *(undefined2 *)(param_1 + param_2) = 0x73;
  puVar5 = local_30;
  if (local_30 == (undefined *)0x0) {
    puVar5 = &DAT_00104b55;
  }
  printf(param_1,puVar5);
  if (local_30 != (undefined *)0x0) {
    freecon(local_30);
  }
  if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
    return cVar1 < '\0';
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: print_statfs
undefined print_statfs(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined param_4,
                      undefined8 param_5,undefined8 param_6,long *param_7) {
  undefined8 uVar1;
  int local_38;
  ulong local_30;
  long local_28;
  
  switch(param_4) {
  case 0x53:
    local_28 = param_7[1];
    if (local_28 == 0) {
      local_28 = *param_7;
    }
    out_uint(param_1,param_2,local_28);
    break;
  case 0x54:
    uVar1 = human_fstype(param_7);
    out_string(param_1,param_2,uVar1);
    break;
  default:
    fputc_unlocked(0x3f,_stdout);
    break;
  case 0x61:
    out_int(param_1,param_2,param_7[4]);
    break;
  case 0x62:
    out_int(param_1,param_2,param_7[2]);
    break;
  case 99:
    out_uint(param_1,param_2,param_7[5]);
    break;
  case 100:
    out_int(param_1,param_2,param_7[6]);
    break;
  case 0x66:
    out_int(param_1,param_2,param_7[3]);
    break;
  case 0x69:
    local_30 = 0;
    for (local_38 = 0; (local_38 < 2 && ((ulong)((long)local_38 * 4) < 8)); local_38 = local_38 + 1)
    {
      local_30 = local_30 |
                 (ulong)*(uint *)((long)param_7 + (long)(1 - local_38) * 4 + 0x40) <<
                 ((byte)(local_38 << 5) & 0x3f);
    }
    out_uint_x(param_1,param_2,local_30);
    break;
  case 0x6c:
    out_uint(param_1,param_2,param_7[10]);
    break;
  case 0x6e:
    out_string(param_1,param_2,param_6);
    break;
  case 0x73:
    out_uint(param_1,param_2,*param_7);
    break;
  case 0x74:
    out_uint_x(param_1,param_2,*(undefined4 *)(param_7 + 0xb));
  }
  return 0;
}

// Function: find_bind_mount
char * find_bind_mount(char *param_1) {
  char cVar1;
  int iVar2;
  undefined8 uVar3;
  int *piVar4;
  long in_FS_OFFSET;
  char *local_160;
  char **local_158;
  stat local_148;
  stat local_b8;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  local_160 = (char *)0x0;
  if (tried_mount_list_1 != '\x01') {
    mount_list_0 = (char **)read_file_system_list(0);
    if (mount_list_0 == (char **)0x0) {
      uVar3 = gettext("cannot read table of mounted file systems");
      piVar4 = __errno_location();
      error(0,*piVar4,&DAT_00104c0a,uVar3);
    }
    tried_mount_list_1 = '\x01';
  }
  iVar2 = stat(param_1,&local_148);
  if (iVar2 == 0) {
    for (local_158 = mount_list_0; local_158 != (char **)0x0; local_158 = (char **)local_158[6]) {
      if (((((*(byte *)(local_158 + 5) & 1) != 0) && (**local_158 == '/')) &&
          (cVar1 = streq(local_158[1],param_1), cVar1 != '\0')) &&
         ((iVar2 = stat(*local_158,&local_b8), iVar2 == 0 &&
          (cVar1 = psame_inode(&local_148,&local_b8), cVar1 != '\0')))) {
        local_160 = *local_158;
        break;
      }
    }
  }
  else {
    local_160 = (char *)0x0;
  }
  if (local_20 == *(long *)(in_FS_OFFSET + 0x28)) {
    return local_160;
  }
                    /* WARNING: Subroutine does not return */
  __stack_chk_fail();
}

// Function: out_mount_point
undefined out_mount_point(char *param_1,undefined8 param_2,undefined8 param_3,long param_4) {
  char *__ptr;
  undefined8 uVar1;
  undefined8 uVar2;
  int *piVar3;
  undefined local_49;
  undefined *local_40;
  undefined *local_38;
  
  local_40 = (undefined *)0x0;
  local_38 = (undefined *)0x0;
  local_49 = 1;
  if ((follow_links != '\0') || ((*(uint *)(param_4 + 0x18) & 0xf000) != 0xa000)) {
    __ptr = canonicalize_file_name(param_1);
    if (__ptr == (char *)0x0) {
      uVar1 = quotearg_style(4,param_1);
      uVar2 = gettext("failed to canonicalize %s");
      piVar3 = __errno_location();
      error(0,*piVar3,uVar2,uVar1);
      goto LAB_0010252e;
    }
    local_40 = (undefined *)find_bind_mount(__ptr);
    free(__ptr);
    if (local_40 != (undefined *)0x0) {
      local_49 = 0;
      goto LAB_0010252e;
    }
  }
  local_38 = (undefined *)find_mount_point(param_1,param_4);
  if (local_38 != (undefined *)0x0) {
    local_40 = (undefined *)find_bind_mount(local_38);
    local_49 = 0;
  }
LAB_0010252e:
  if ((local_40 == (undefined *)0x0) && (local_40 = local_38, local_38 == (undefined *)0x0)) {
    local_40 = &DAT_00104b55;
  }
  out_string(param_2,param_3,local_40);
  free(local_38);
  return local_49;
}

// Function: neg_to_zero
undefined  [16] neg_to_zero(undefined8 param_1,long param_2) {
  undefined auVar1 [16];
  
  if (param_2 < 0) {
    param_1 = 0;
    param_2 = 0;
  }
  auVar1._8_8_ = param_2;
  auVar1._0_8_ = param_1;
  return auVar1;
}

// Function: getenv_quoting_style
void getenv_quoting_style(void) {
  int iVar1;
  char *pcVar2;
  undefined8 uVar3;
  undefined8 uVar4;
  
  pcVar2 = getenv("QUOTING_STYLE");
  if (pcVar2 == (char *)0x0) {
    set_quoting_style(0,4);
  }
  else {
    iVar1 = argmatch(pcVar2,&quoting_style_args,&quoting_style_vals,4);
    if (iVar1 < 0) {
      set_quoting_style(0,4);
      uVar3 = quote(pcVar2);
      uVar4 = gettext("ignoring invalid value of environment variable QUOTING_STYLE: %s");
      error(0,0,uVar4,uVar3);
    }
    else {
      set_quoting_style(0,*(undefined4 *)(&quoting_style_vals + (long)iVar1 * 4));
    }
  }
  return;
}

// Function: print_esc_char
void print_esc_char(char param_1) {
  int iVar1;
  undefined8 uVar2;
  char local_2c;
  
  iVar1 = (int)param_1;
  local_2c = param_1;
  if (iVar1 == 0x22) goto LAB_0010276d;
  if (((iVar1 < 0x22) || (0x76 < iVar1)) || (iVar1 < 0x5c)) {
switchD_001026f9_caseD_5d:
    uVar2 = gettext("warning: unrecognized escape \'\\%c\'");
    error(0,0,uVar2,(int)param_1);
  }
  else {
    switch(iVar1) {
    case 0x5c:
      break;
    default:
      goto switchD_001026f9_caseD_5d;
    case 0x61:
      local_2c = '\a';
      break;
    case 0x62:
      local_2c = '\b';
      break;
    case 0x65:
      local_2c = '\x1b';
      break;
    case 0x66:
      local_2c = '\f';
      break;
    case 0x6e:
      local_2c = '\n';
      break;
    case 0x72:
      local_2c = '\r';
      break;
    case 0x74:
      local_2c = '\t';
      break;
    case 0x76:
      local_2c = '\v';
    }
  }
LAB_0010276d:
  putchar_unlocked((int)local_2c);
  return;
}

// Function: format_code_offset
long format_code_offset(long param_1) {
  size_t sVar1;
  char *local_18;
  
  sVar1 = strspn((char *)(param_1 + 1),"\'-+ #0I");
  local_18 = (char *)(param_1 + sVar1 + 1);
  sVar1 = strspn(local_18,digits);
  local_18 = local_18 + sVar1;
  if (*local_18 == '.') {
    sVar1 = strspn(local_18 + 1,digits);
    local_18 = local_18 + sVar1 + 1;
  }
  return (long)local_18 - param_1;
}

// Function: print_it
bool print_it(char *param_1,undefined4 param_2,undefined8 param_3,code *param_4,undefined8 param_5) {
  char *pcVar1;
  byte bVar2;
  char cVar3;
  int iVar4;
  size_t sVar5;
  void *__dest;
  ulong __n;
  undefined8 uVar6;
  undefined8 uVar7;
  bool local_67;
  char local_66;
  char local_65;
  int local_64;
  int local_60;
  int local_5c;
  char *local_48;
  
  local_67 = false;
  sVar5 = strlen(param_1);
  __dest = (void *)xmalloc(sVar5 + 3);
  local_48 = param_1;
  do {
    if (*local_48 == '\0') {
      free(__dest);
      fputs_unlocked(trailing_delim,_stdout);
      return local_67;
    }
    if (*local_48 == '%') {
      __n = format_code_offset(local_48);
      cVar3 = local_48[__n];
      local_65 = '\0';
      memcpy(__dest,local_48,__n);
      local_48 = local_48 + __n;
      local_66 = cVar3;
      if (cVar3 == 'L') {
LAB_0010298b:
        local_66 = local_48[1];
        if ((param_4 == print_stat) && ((local_66 == 'd' || (local_66 == 'r')))) {
          local_48 = local_48 + 1;
          local_65 = cVar3;
        }
        else {
          local_65 = '\0';
          local_66 = cVar3;
        }
      }
      else if (cVar3 < 'M') {
        if (cVar3 == 'H') goto LAB_0010298b;
        if (cVar3 < 'I') {
          if (cVar3 == '\0') {
            local_48 = local_48 + -1;
          }
          else if (cVar3 != '%') goto LAB_001029cb;
          if (1 < __n) {
            *(char *)((long)__dest + __n) = cVar3;
            *(undefined *)((long)__dest + __n + 1) = 0;
            uVar7 = quote(__dest);
            uVar6 = gettext("%s: invalid directive");
            error(1,0,uVar6,uVar7);
          }
          putchar_unlocked(0x25);
          goto LAB_00102c71;
        }
      }
LAB_001029cb:
      bVar2 = (*param_4)(__dest,__n,(int)local_65,(int)local_66,param_2,param_3,param_5);
      local_67 = (bVar2 | local_67) != 0;
    }
    else if (*local_48 == '\\') {
      if (interpret_backslash_escapes == '\x01') {
        pcVar1 = local_48 + 1;
        if ((*pcVar1 < '0') || ('7' < *pcVar1)) {
          if ((*pcVar1 == 'x') && (cVar3 = c_isxdigit((int)local_48[2]), cVar3 != '\0')) {
            if ((local_48[2] < 'a') || ('f' < local_48[2])) {
              if ((local_48[2] < 'A') || ('F' < local_48[2])) {
                local_5c = local_48[2] + -0x30;
              }
              else {
                local_5c = local_48[2] + -0x37;
              }
            }
            else {
              local_5c = local_48[2] + -0x57;
            }
            cVar3 = c_isxdigit((int)local_48[3]);
            pcVar1 = local_48 + 2;
            if (cVar3 != '\0') {
              pcVar1 = local_48 + 3;
              if ((*pcVar1 < 'a') || ('f' < *pcVar1)) {
                if ((*pcVar1 < 'A') || ('F' < *pcVar1)) {
                  iVar4 = *pcVar1 + -0x30;
                }
                else {
                  iVar4 = *pcVar1 + -0x37;
                }
              }
              else {
                iVar4 = *pcVar1 + -0x57;
              }
              local_5c = iVar4 + local_5c * 0x10;
            }
            local_48 = pcVar1;
            putchar_unlocked(local_5c);
          }
          else if (*pcVar1 == '\0') {
            uVar7 = gettext("warning: backslash at end of format");
            error(0,0,uVar7);
            putchar_unlocked(0x5c);
          }
          else {
            print_esc_char((int)*pcVar1);
            local_48 = pcVar1;
          }
        }
        else {
          local_64 = *pcVar1 + -0x30;
          local_60 = 1;
          for (local_48 = local_48 + 2; ((local_60 < 3 && ('/' < *local_48)) && (*local_48 < '8'));
              local_48 = local_48 + 1) {
            local_64 = *local_48 + -0x30 + local_64 * 8;
            local_60 = local_60 + 1;
          }
          putchar_unlocked(local_64);
          local_48 = local_48 + -1;
        }
      }
      else {
        putchar_unlocked(0x5c);
      }
    }
    else {
      putchar_unlocked((int)*local_48);
    }
LAB_00102c71:
    local_48 = local_48 + 1;
  } while( true );
}

// Function: do_statfs
bool do_statfs(char *param_1,undefined8 param_2) {
  char cVar1;
  int iVar2;
  undefined8 uVar3;
  undefined8 uVar4;
  int *piVar5;
  long in_FS_OFFSET;
  bool bVar6;
  statvfs local_98;
  long local_20;
  
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  cVar1 = streq(param_1,&DAT_00104afb);
  if (cVar1 == '\0') {
    iVar2 = statvfs(param_1,&local_98);
    if (iVar2 == 0) {
      cVar1 = print_it(param_2,0xffffffff,param_1,print_statfs,&local_98);
      bVar6 = cVar1 == '\0';
    }
    else {
      uVar3 = quotearg_style(4,param_1);
      uVar4 = gettext("cannot read file system information for %s");
      piVar5 = __errno_location();
      error(0,*piVar5,uVar4,uVar3);
      bVar6 = false;
    }
  }
  else {
    uVar3 = quotearg_style(4,param_1);
    uVar4 = gettext("using %s to denote standard input does not work in file system mode");
    error(0,0,uVar4,uVar3);
    bVar6 = false;
  }
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return bVar6;
}

// Function: fmt_to_mask
undefined8 fmt_to_mask(undefined param_1) {
  undefined8 uVar1;
  
  switch(param_1) {
  case 0x41:
  case 0x61:
    uVar1 = 2;
    break;
  default:
    uVar1 = 0;
    break;
  case 0x44:
  case 100:
    uVar1 = 2;
    break;
  case 0x46:
    uVar1 = 1;
    break;
  case 0x47:
  case 0x67:
    uVar1 = 0x10;
    break;
  case 0x4e:
    uVar1 = 2;
    break;
  case 0x54:
  case 0x74:
    uVar1 = 2;
    break;
  case 0x55:
  case 0x75:
    uVar1 = 8;
    break;
  case 0x57:
  case 0x77:
    uVar1 = 0x800;
    break;
  case 0x58:
  case 0x78:
    uVar1 = 0x20;
    break;
  case 0x59:
  case 0x79:
    uVar1 = 0x40;
    break;
  case 0x5a:
  case 0x7a:
    uVar1 = 0x80;
    break;
  case 0x62:
    uVar1 = 0x400;
    break;
  case 0x66:
    uVar1 = 3;
    break;
  case 0x68:
    uVar1 = 4;
    break;
  case 0x69:
    uVar1 = 0x100;
    break;
  case 0x6d:
    uVar1 = 0x102;
    break;
  case 0x73:
    uVar1 = 0x200;
  }
  return uVar1;
}

// Function: format_to_mask
uint format_to_mask(char *param_1) {
  uint uVar1;
  long lVar2;
  uint local_14;
  char *local_10;
  
  local_14 = 0;
  local_10 = param_1;
  do {
    if (*local_10 == '\0') {
      return local_14;
    }
    if (*local_10 == '%') {
      lVar2 = format_code_offset(local_10);
      local_10 = local_10 + lVar2;
      if (*local_10 == '\0') {
        return local_14;
      }
      uVar1 = fmt_to_mask((int)*local_10);
      local_14 = local_14 | uVar1;
    }
    local_10 = local_10 + 1;
  } while( true );
}

// Function: do_stat
bool do_stat(undefined *param_1,undefined8 param_2,undefined8 param_3) {
  char cVar1;
  undefined4 uVar2;
  int iVar3;
  undefined8 uVar4;
  int *piVar5;
  undefined8 uVar6;
  long lVar7;
  undefined8 *puVar8;
  long in_FS_OFFSET;
  bool bVar9;
  byte bVar10;
  undefined8 local_228;
  uint local_208;
  undefined *local_1f0;
  undefined *local_1d8;
  undefined local_1d0 [24];
  undefined local_1b8 [144];
  undefined8 local_128 [3];
  ushort local_10c;
  undefined8 local_d8;
  undefined8 local_d0;
  long local_20;
  
  bVar10 = 0;
  local_20 = *(long *)(in_FS_OFFSET + 0x28);
  cVar1 = streq(param_1,&DAT_00104afb);
  if (cVar1 == '\0') {
    iVar3 = -100;
  }
  else {
    iVar3 = 0;
  }
  local_208 = 0;
  puVar8 = local_128;
  for (lVar7 = 0x20; lVar7 != 0; lVar7 = lVar7 + -1) {
    *puVar8 = 0;
    puVar8 = puVar8 + (ulong)bVar10 * -2 + 1;
  }
  local_1d8 = local_1b8;
  local_1d0._0_8_ = 0xffffffffffffffff;
  local_1d0._8_8_ = 0xffffffffffffffff;
  if (iVar3 == -100) {
    local_1f0 = param_1;
    if (follow_links != '\x01') {
      local_208 = 0x100;
    }
  }
  else {
    local_1f0 = &DAT_001045b4;
    local_208 = 0x1000;
  }
  if (dont_sync == '\0') {
    if (force_sync != '\0') {
      local_208 = local_208 | 0x2000;
    }
  }
  else {
    local_208 = local_208 | 0x4000;
  }
  if (force_sync != '\x01') {
    local_208 = local_208 | 0x800;
  }
  uVar2 = format_to_mask(param_2);
  iVar3 = statx(iVar3,local_1f0,local_208,uVar2,local_128);
  if (iVar3 < 0) {
    if ((local_208 & 0x1000) == 0) {
      uVar4 = quotearg_style(4,param_1);
      uVar6 = gettext("cannot statx %s");
      piVar5 = __errno_location();
      error(0,*piVar5,uVar6,uVar4);
    }
    else {
      uVar4 = gettext("cannot stat standard input");
      piVar5 = __errno_location();
      error(0,*piVar5,uVar4);
    }
    bVar9 = false;
  }
  else {
    if (((local_10c & 0xf000) == 0x6000) || (local_228 = param_2, (local_10c & 0xf000) == 0x2000)) {
      local_228 = param_3;
    }
    statx_to_stat(local_128,local_1b8);
    if (((uint)local_128[0] & 0x800) != 0) {
      local_1d0._0_16_ = statx_timestamp_to_timespec(local_d8,local_d0);
    }
    cVar1 = print_it(local_228,iVar3,param_1,print_stat,&local_1d8);
    bVar9 = cVar1 == '\0';
  }
  if (local_20 != *(long *)(in_FS_OFFSET + 0x28)) {
                    /* WARNING: Subroutine does not return */
    __stack_chk_fail();
  }
  return bVar9;
}

// Function: unsigned_file_size
undefined8 unsigned_file_size(undefined8 param_1) {
  return param_1;
}

// Function: print_stat
bool print_stat(undefined8 param_1,undefined8 param_2,char param_3,undefined param_4,
               undefined8 param_5,undefined8 param_6,ulonglong **param_7) {
  ulonglong *puVar1;
  ulonglong *puVar2;
  char cVar3;
  undefined4 uVar4;
  uint uVar5;
  void *__ptr;
  undefined8 uVar6;
  int *piVar7;
  passwd *ppVar8;
  char *pcVar9;
  group *pgVar10;
  undefined8 uVar11;
  ulonglong uVar12;
  undefined auVar13 [16];
  bool local_61;
  
  puVar1 = *param_7;
  puVar2 = param_7[2];
  local_61 = false;
  switch(param_4) {
  case 0x41:
    uVar11 = human_access(puVar1);
    out_string(param_1,param_2,uVar11);
    break;
  case 0x42:
    out_uint(param_1,param_2,0x200);
    break;
  case 0x43:
    cVar3 = out_file_context(param_1,param_2,param_6);
    local_61 = cVar3 != '\0';
    break;
  case 0x44:
    out_uint_x(param_1,param_2,*puVar1);
    break;
  default:
    fputc_unlocked(0x3f,_stdout);
    break;
  case 0x46:
    uVar11 = file_type(puVar1);
    out_string(param_1,param_2,uVar11);
    break;
  case 0x47:
    pgVar10 = getgrgid(*(__gid_t *)(puVar1 + 4));
    if (pgVar10 == (group *)0x0) {
      pcVar9 = "UNKNOWN";
    }
    else {
      pcVar9 = pgVar10->gr_name;
    }
    out_string(param_1,param_2,pcVar9);
    break;
  case 0x4e:
    uVar4 = get_quoting_style(0);
    uVar11 = quotearg_style(uVar4,param_6);
    out_string(param_1,param_2,uVar11);
    if ((*(uint *)(puVar1 + 3) & 0xf000) == 0xa000) {
      __ptr = (void *)areadlink_with_size(param_6,puVar1[6]);
      if (__ptr == (void *)0x0) {
        uVar11 = quotearg_style(4,param_6);
        uVar6 = gettext("cannot read symbolic link %s");
        piVar7 = __errno_location();
        error(0,*piVar7,uVar6,uVar11);
        local_61 = true;
      }
      else {
        printf(" -> ");
        uVar4 = get_quoting_style(0);
        uVar11 = quotearg_style(uVar4,__ptr);
        out_string(param_1,param_2,uVar11);
        free(__ptr);
      }
    }
    break;
  case 0x52:
    out_uint_x(param_1,param_2,puVar1[5]);
    break;
  case 0x54:
    uVar5 = gnu_dev_minor(puVar1[5]);
    out_uint_x(param_1,param_2,uVar5);
    break;
  case 0x55:
    ppVar8 = getpwuid(*(__uid_t *)((long)puVar1 + 0x1c));
    if (ppVar8 == (passwd *)0x0) {
      pcVar9 = "UNKNOWN";
    }
    else {
      pcVar9 = ppVar8->pw_name;
    }
    out_string(param_1,param_2,pcVar9);
    break;
  case 0x57:
    auVar13 = neg_to_zero(param_7[1],puVar2);
    out_epoch_sec(param_1,param_2,auVar13._0_8_,auVar13._8_8_);
    break;
  case 0x58:
    auVar13 = get_stat_atime(puVar1);
    out_epoch_sec(param_1,param_2,auVar13._0_8_,auVar13._8_8_);
    break;
  case 0x59:
    auVar13 = get_stat_mtime(puVar1);
    out_epoch_sec(param_1,param_2,auVar13._0_8_,auVar13._8_8_);
    break;
  case 0x5a:
    auVar13 = get_stat_ctime(puVar1);
    out_epoch_sec(param_1,param_2,auVar13._0_8_,auVar13._8_8_);
    break;
  case 0x61:
    out_uint_o(param_1,param_2,*(uint *)(puVar1 + 3) & 0xfff);
    break;
  case 0x62:
    out_uint(param_1,param_2,puVar1[8]);
    break;
  case 100:
    if (param_3 == 'H') {
      uVar5 = gnu_dev_major(*puVar1);
      out_uint(param_1,param_2,uVar5);
    }
    else if (param_3 == 'L') {
      uVar5 = gnu_dev_minor(*puVar1);
      out_uint(param_1,param_2,uVar5);
    }
    else {
      out_uint(param_1,param_2,*puVar1);
    }
    break;
  case 0x66:
    out_uint_x(param_1,param_2,*(undefined4 *)(puVar1 + 3));
    break;
  case 0x67:
    out_uint(param_1,param_2,*(undefined4 *)(puVar1 + 4));
    break;
  case 0x68:
    out_uint(param_1,param_2,puVar1[2]);
    break;
  case 0x69:
    out_uint(param_1,param_2,puVar1[1]);
    break;
  case 0x6d:
    cVar3 = out_mount_point(param_6,param_1,param_2,puVar1);
    local_61 = cVar3 != '\0';
    break;
  case 0x6e:
    out_string(param_1,param_2,param_6);
    break;
  case 0x6f:
    if (((long)puVar1[7] < 1) || (0x2000000000000000 < puVar1[7])) {
      uVar12 = 0x200;
    }
    else {
      uVar12 = puVar1[7];
    }
    out_uint(param_1,param_2,uVar12);
    break;
  case 0x72:
    if (param_3 == 'H') {
      uVar5 = gnu_dev_major(puVar1[5]);
      out_uint(param_1,param_2,uVar5);
    }
    else if (param_3 == 'L') {
      uVar5 = gnu_dev_minor(puVar1[5]);
      out_uint(param_1,param_2,uVar5);
    }
    else {
      out_uint(param_1,param_2,puVar1[5]);
    }
    break;
  case 0x73:
    uVar11 = unsigned_file_size(puVar1[6]);
    out_uint(param_1,param_2,uVar11);
    break;
  case 0x74:
    uVar5 = gnu_dev_major(puVar1[5]);
    out_uint_x(param_1,param_2,uVar5);
    break;
  case 0x75:
    out_uint(param_1,param_2,*(undefined4 *)((long)puVar1 + 0x1c));
    break;
  case 0x77:
    if ((long)puVar2 < 0) {
      out_string(param_1,param_2,&DAT_00104afb);
    }
    else {
      uVar11 = human_time(param_7[1],puVar2);
      out_string(param_1,param_2,uVar11);
    }
    break;
  case 0x78:
    auVar13 = get_stat_atime(puVar1);
    uVar11 = auVar13._0_8_;
    uVar11 = human_time(uVar11,auVar13._8_8_,uVar11,uVar11);
    out_string(param_1,param_2,uVar11);
    break;
  case 0x79:
    auVar13 = get_stat_mtime(puVar1);
    uVar11 = auVar13._0_8_;
    uVar11 = human_time(uVar11,auVar13._8_8_,uVar11,uVar11);
    out_string(param_1,param_2,uVar11);
    break;
  case 0x7a:
    auVar13 = get_stat_ctime(puVar1);
    uVar11 = auVar13._0_8_;
    uVar11 = human_time(uVar11,auVar13._8_8_,uVar11,uVar11);
    out_string(param_1,param_2,uVar11);
  }
  return local_61;
}

// Function: default_format
undefined8 default_format(char param_1,char param_2,char param_3) {
  undefined8 uVar1;
  void *pvVar2;
  void *local_18;
  
  if (param_1 == '\0') {
    if (param_2 == '\0') {
      uVar1 = gettext("  File: %N\n  Size: %-10s\tBlocks: %-10b IO Block: %-6o %F\n");
      pvVar2 = (void *)xstrdup(uVar1);
      if (param_3 == '\0') {
        uVar1 = gettext("Device: %Hd,%Ld\tInode: %-10i  Links: %h\n");
        local_18 = (void *)xasprintf(&DAT_00105120,pvVar2,uVar1);
      }
      else {
        uVar1 = gettext("Device: %Hd,%Ld\tInode: %-10i  Links: %-5h Device type: %Hr,%Lr\n");
        local_18 = (void *)xasprintf(&DAT_00105120,pvVar2,uVar1);
      }
      free(pvVar2);
      uVar1 = gettext("Access: (%04a/%10.10A)  Uid: (%5u/%8U)   Gid: (%5g/%8G)\n");
      pvVar2 = (void *)xasprintf(&DAT_00105120,local_18,uVar1);
      free(local_18);
      uVar1 = gettext("Access: %x\nModify: %y\nChange: %z\n Birth: %w\n");
      local_18 = (void *)xasprintf(&DAT_00105120,pvVar2,uVar1);
      free(pvVar2);
    }
    else {
      local_18 = (void *)xstrdup("%n %s %b %f %u %g %D %i %h %t %T %X %Y %Z %W %o\n");
    }
  }
  else if (param_2 == '\0') {
    uVar1 = gettext(
                   "  File: \"%n\"\n    ID: %-8i Namelen: %-7l Type: %T\nBlock size: %-10s Fundamental block size: %S\nBlocks: Total: %-10b Free: %-10f Available: %a\nInodes: Total: %-10c Free: %d\n"
                   );
    local_18 = (void *)xstrdup(uVar1);
  }
  else {
    local_18 = (void *)xstrdup("%n %i %l %t %s %S %b %f %a %c %d\n");
  }
  return local_18;
}

// Function: usage
void usage(int param_1) {
  FILE *pFVar1;
  undefined8 uVar2;
  char *pcVar3;
  
  uVar2 = _program_name;
  if (param_1 == 0) {
    pcVar3 = (char *)gettext("Usage: %s [OPTION]... FILE...\n");
    printf(pcVar3,uVar2);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("Display file or file system status.\n");
    fputs_unlocked(pcVar3,pFVar1);
    emit_mandatory_arg_note();
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -L, --dereference     follow links\n  -f, --file-system     display file system status instead of file status\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "      --cached=MODE     specify how to use cached attributes;\n                          useful on remote file systems. See MODE below\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  -c  --format=FORMAT   use the specified FORMAT instead of the default;\n                          output a newline after each use of FORMAT\n      --printf=FORMAT   like --format, but interpret backslash escapes,\n                          and do not output a mandatory trailing newline;\n                          if you want a newline, include \\n in FORMAT\n  -t, --terse           print the information in terse form\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --help        display this help and exit\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext("      --version     output version information and exit\n");
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "\nThe MODE argument of --cached can be: always, never, or default.\n\'always\' will use cached attributes if available, while\n\'never\' will try to synchronize with the latest attributes, and\n\'default\' will leave it up to the underlying file system.\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "\nThe valid format sequences for files (without --file-system):\n\n  %a   permission bits in octal (see \'#\' and \'0\' printf flags)\n  %A   permission bits and file type in human readable form\n  %b   number of blocks allocated (see %B)\n  %B   the size in bytes of each block reported by %b\n  %C   SELinux security context string\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  %d   device number in decimal (st_dev)\n  %D   device number in hex (st_dev)\n  %Hd  major device number in decimal\n  %Ld  minor device number in decimal\n  %f   raw mode in hex\n  %F   file type\n  %g   group ID of owner\n  %G   group name of owner\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  %h   number of hard links\n  %i   inode number\n  %m   mount point\n  %n   file name\n  %N   quoted file name with dereference if symbolic link\n  %o   optimal I/O transfer size hint\n  %s   total size, in bytes\n  %r   device type in decimal (st_rdev)\n  %R   device type in hex (st_rdev)\n  %Hr  major device type in decimal, for character/block device special files\n  %Lr  minor device type in decimal, for character/block device special files\n  %t   major device type in hex, for character/block device special files\n  %T   minor device type in hex, for character/block device special files\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  %u   user ID of owner\n  %U   user name of owner\n  %w   time of file birth, human-readable; - if unknown\n  %W   time of file birth, seconds since Epoch; 0 if unknown\n  %x   time of last access, human-readable\n  %X   time of last access, seconds since Epoch\n  %y   time of last data modification, human-readable\n  %Y   time of last data modification, seconds since Epoch\n  %z   time of last status change, human-readable\n  %Z   time of last status change, seconds since Epoch\n\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "Valid format sequences for file systems:\n\n  %a   free blocks available to non-superuser\n  %b   total data blocks in file system\n  %c   total file nodes in file system\n  %d   free file nodes in file system\n  %f   free blocks in file system\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pFVar1 = _stdout;
    pcVar3 = (char *)gettext(
                            "  %i   file system ID in hex\n  %l   maximum length of filenames\n  %n   file name\n  %s   block size (for faster transfers)\n  %S   fundamental block size (for block counts)\n  %t   file system type in hex\n  %T   file system type in human readable form\n"
                            );
    fputs_unlocked(pcVar3,pFVar1);
    pcVar3 = (char *)gettext("\n--terse is equivalent to the following FORMAT:\n    %s");
    printf(pcVar3,"%n %s %b %f %u %g %D %i %h %t %T %X %Y %Z %W %o\n");
    pcVar3 = (char *)gettext("--terse --file-system is equivalent to the following FORMAT:\n    %s")
    ;
    printf(pcVar3,"%n %i %l %t %s %S %b %f %a %c %d\n");
    pcVar3 = (char *)gettext(
                            "\nYour shell may have its own version of %s, which usually supersedes\nthe version described here.  Please refer to your shell\'s documentation\nfor details about the options it supports.\n"
                            );
    printf(pcVar3,&DAT_00105fd9);
    emit_ancillary_info(&DAT_00105fd9);
  }
  else {
    pcVar3 = (char *)gettext("Try \'%s --help\' for more information.\n");
    fprintf(_stderr,pcVar3,uVar2);
  }
                    /* WARNING: Subroutine does not return */
  exit(param_1);
}

// Function: main
void main(int param_1,undefined8 *param_2) {
  uint uVar1;
  byte bVar2;
  int iVar3;
  lconv *plVar4;
  long lVar5;
  undefined8 uVar6;
  char *pcVar7;
  char local_3b;
  undefined local_3a;
  bool local_39;
  int local_38;
  char *local_28;
  char *local_20;
  
  local_3b = '\0';
  local_3a = 0;
  local_28 = (char *)0x0;
  local_39 = true;
  set_program_name(*param_2);
  setlocale(6,"");
  bindtextdomain("coreutils","/usr/local/share/locale");
  textdomain("coreutils");
  plVar4 = localeconv();
  if (*plVar4->decimal_point == '\0') {
    decimal_point = ".";
  }
  else {
    decimal_point = plVar4->decimal_point;
  }
  decimal_point_len = strlen(decimal_point);
  atexit((__func *)&close_stdout);
LAB_00104216:
  while( true ) {
    iVar3 = getopt_long(param_1,param_2,"c:fLt",long_options,0);
    if (iVar3 == -1) {
      if (param_1 == _optind) {
        uVar6 = gettext("missing operand");
        error(0,0,uVar6);
        usage(1);
      }
      if (local_28 == (char *)0x0) {
        local_28 = (char *)default_format(local_3b,local_3a,0);
        local_20 = (char *)default_format(local_3b,local_3a,1);
      }
      else {
        pcVar7 = strstr(local_28,"%N");
        if (pcVar7 != (char *)0x0) {
          getenv_quoting_style();
        }
        local_20 = local_28;
      }
      for (local_38 = _optind; local_38 < param_1; local_38 = local_38 + 1) {
        if (local_3b == '\0') {
          bVar2 = do_stat(param_2[local_38],local_28,local_20);
        }
        else {
          bVar2 = do_statfs(param_2[local_38],local_28);
        }
        local_39 = (bVar2 & local_39) != 0;
      }
                    /* WARNING: Subroutine does not return */
      exit((uint)(local_39 ^ 1));
    }
    if (iVar3 != 0x80) break;
    local_28 = _optarg;
    interpret_backslash_escapes = 1;
    trailing_delim = &DAT_001045b4;
  }
  if (iVar3 < 0x81) {
    if (iVar3 == 0x74) {
      local_3a = 1;
      goto LAB_00104216;
    }
    if (0x74 < iVar3) goto LAB_0010420b;
    if (iVar3 == 0x66) {
      local_3b = '\x01';
      goto LAB_00104216;
    }
    if (iVar3 < 0x67) {
      if (iVar3 == 99) {
        local_28 = _optarg;
        interpret_backslash_escapes = 0;
        trailing_delim = &DAT_00105ff8;
        goto LAB_00104216;
      }
      if (iVar3 < 100) {
        if (iVar3 == 0x4c) {
          follow_links = 1;
          goto LAB_00104216;
        }
        if (iVar3 < 0x4d) {
          if (iVar3 == 0) {
            lVar5 = __xargmatch_internal
                              ("--cached",_optarg,cached_args,cached_modes,4,_argmatch_die,1);
            uVar1 = *(uint *)(cached_modes + lVar5 * 4);
            if (uVar1 == 2) {
              force_sync = 0;
              dont_sync = 1;
            }
            else if (uVar1 < 3) {
              if (uVar1 == 0) {
                force_sync = 0;
                dont_sync = 0;
              }
              else if (uVar1 == 1) {
                force_sync = 1;
                dont_sync = 0;
              }
            }
            goto LAB_00104216;
          }
          if (iVar3 < 1) {
            if (iVar3 == -0x83) {
LAB_001041b0:
              uVar6 = proper_name_lite("Michael Meskes","Michael Meskes");
              version_etc(_stdout,&DAT_00105fd9,"GNU coreutils",_Version,uVar6,0);
                    /* WARNING: Subroutine does not return */
              exit(0);
            }
            if (iVar3 == -0x82) {
              usage(0);
              goto LAB_001041b0;
            }
          }
        }
      }
    }
  }
LAB_0010420b:
  usage(1);
  goto LAB_00104216;
}

